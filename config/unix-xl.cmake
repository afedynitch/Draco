#-----------------------------*-cmake-*----------------------------------------#
# file   config/unix-xl.cmake
# author Gabriel Rockefeller, Kelly Thompson <kgt@lanl.gov>
# date   2012 Nov 1
# brief  Establish flags for Linux64 - IBM XL C++
# note   Copyright (C) 2016-2020 Triad National Security, LLC.
#        All rights reserved.
#------------------------------------------------------------------------------#

# Ref:
# https://www.ibm.com/support/knowledgecenter/en/SSXVZZ_16.1.1/com.ibm.xlcpp1611.lelinux.doc/compiler_ref/rucmpopt.html
#
# Compiler flag checks
#
include(platform_checks)
query_openmp_availability()

#
# Compiler Flags
#

if( NOT CXX_FLAGS_INITIALIZED )
   set( CXX_FLAGS_INITIALIZED "yes" CACHE INTERNAL "using draco settings." )

  set( CMAKE_C_FLAGS             "-qflttrap")
  if( EXISTS /usr/gapps )
    # ATS-2
    string( APPEND CMAKE_C_FLAGS " --gcc-toolchain=/usr/tce/packages/gcc/gcc-8.3.1" )
  elseif(EXISTS /projects/opt/ppc64le/ibm )
    # Darwin power9 - extract version from module environment.
    string(REPLACE ":" ";" modules $ENV{LOADEDMODULES})
    foreach( module ${modules} )
      if( ${module} MATCHES "^gcc" )
        string( REGEX REPLACE
          "[^0-9]*([0-9]+).([0-9]+).([0-9]+)" "\\1.\\2.\\3"
          gcc_version  ${module} )
      elseif( NOT DEFINED xlc_version AND ${module} MATCHES "^ibm/xlc" )
        string( REGEX REPLACE
          "[^0-9]*([0-9]+).([0-9]+).([0-9]+).([0-9]+).*" "\\1.\\2.\\3.\\4"
          xlc_version ${module} )
        string( REGEX REPLACE "[^0-9]*([0-9]+).([0-9]+).([0-9]+).*" "\\1.\\2.\\3"
          xlc_version_3 ${xlc_version} )
#      Only cuda-10.1 is currently supported
      elseif( ${module} MATCHES "^cuda" )
        string( REGEX REPLACE
          "[^0-9]*([0-9]+).([0-9]+)" "\\1.\\2"
          cuda_version ${module} )
        if( NOT ${cuda_version} STREQUAL "10.1" )
          message( FATAL_ERROR "Only cuda/10.1 is currently supported."
            " Found cuda/${cuda_version} in your environment. Replace your cuda"
            " module with cuda/10.1 or unload the cuda module.")
        endif()
      endif()
    endforeach()
    # Only redhat 7.7 is currently supported
    # file( READ /etc/redhat-release rhr )
    #string( REGEX REPLACE "[^0-9]*([0-9]+).([0-9]+).*" "\\1.\\2"
    #   redhat_version  ${rhr} )
    set( config_file "/projects/opt/ppc64le/ibm/xlc-${xlc_version}/xlC/${xlc_version_3}/etc/xlc.cfg.rhel.7.7.gcc.${gcc_version}.cuda.10.1" )
    if( EXISTS ${config_file} )
      string( APPEND CMAKE_C_FLAGS " -F${config_file}" )
    else()
      message( FATAL_ERROR
        "IBM XLC selected (Darwin), but requested config file was not found."
        "\nconfig_file = ${config_file}" )
    endif()
    unset( gcc_version )
    unset( xl_version )
    unset( config_file )
  endif()

  set( CMAKE_C_FLAGS_DEBUG          "-g -O0 -qsmp=omp:noopt -qfullpath -DDEBUG")
  set( CMAKE_C_FLAGS_RELWITHDEBINFO
    "-g -O2 -qsmp=omp -qstrict=nans:operationprecision -qmaxmem=-1" )
  set( CMAKE_C_FLAGS_RELEASE
    "-O2 -qsmp=omp -qstrict=nans:operationprecision -qmaxmem=-1 -DNDEBUG" )
  set( CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_C_FLAGS_RELEASE}" )

  if( ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "ppc64le")
    string( APPEND CMAKE_C_FLAGS " -qarch=pwr9 -qtune=pwr9" )
  endif()

   # Email from Roy Musselman <roymuss@us.ibm.com, 2019-03-21:
   # For C++14, add -qxflag=disable__cplusplusOverride
   set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -qxflag=disable__cplusplusOverride -Wno-undefined-var-template")
   set( CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}")
   set( CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}")
   set( CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_RELEASE}")
   set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}" )

endif()

##---------------------------------------------------------------------------##
# Ensure cache values always match current selection
##---------------------------------------------------------------------------##
set( CMAKE_C_FLAGS                "${CMAKE_C_FLAGS}"                CACHE STRING
  "compiler flags" FORCE )
set( CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}"          CACHE STRING
  "compiler flags" FORCE )
set( CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}"        CACHE STRING
  "compiler flags" FORCE )
set( CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_C_FLAGS_MINSIZEREL}"     CACHE STRING
  "compiler flags" FORCE )
set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}" CACHE STRING
  "compiler flags" FORCE )

set( CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS}"                CACHE
  STRING "compiler flags" FORCE )
set( CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}"          CACHE
  STRING "compiler flags" FORCE )
set( CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}"        CACHE
  STRING "compiler flags" FORCE )
set( CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}"     CACHE
  STRING "compiler flags" FORCE )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE
  STRING "compiler flags" FORCE )

#toggle_compiler_flag( DRACO_SHARED_LIBS "-qnostaticlink" "EXE_LINKER" "")

# CMake will set OpenMP_C_FLAGS to '-qsmp.'  This option turns on
# OpenMP but also activates the auto-parallelizer.  We don't want to
# enable the 2nd feature so we need to specify the OpenMP flag to be
# '-qsmp=omp.'
if( CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0 )
  toggle_compiler_flag( OPENMP_FOUND             "-qsmp=omp" "C;CXX;EXE_LINKER"
    "" )
#else()
  # toggle_compiler_flag( OPENMP_FOUND             "-qsmp=noauto"
  # "C;CXX;EXE_LINKER" "" )
endif()

#------------------------------------------------------------------------------#
# End config/unix-xl.cmake
#------------------------------------------------------------------------------#
