#-----------------------------*-cmake-*----------------------------------------#
# file   config/unix-gfortran.cmake
# author Kelly Thompson 
# date   2010 Sep 27
# brief  Establish flags for Unix/Linux - Gnu Fortran
# note   Copyright (C) 2010-2013 Los Alamos National Security, LLC.
#        All rights reserved.
#------------------------------------------------------------------------------#
# $Id$
#------------------------------------------------------------------------------#

# Let anyone who is interested in which FORTRAN compiler we're using
# switch on this macro.
set( CMAKE_Fortran_COMPILER_FLAVOR "GFORTRAN" )

# gfortran < 4.3 won't compile Draco
# gfortran < 4.7 won't compile Jayenne
if( "${CMAKE_Fortran_COMPILER_VERSION}x" STREQUAL "x" )
   execute_process(
      COMMAND ${CMAKE_Fortran_COMPILER} --version
      OUTPUT_VARIABLE CMAKE_Fortran_COMPILER_VERSION
      ERROR_QUIET )
   string( REGEX REPLACE "^(.*).Copyright.*" "\\1" 
      CMAKE_Fortran_COMPILER_VERSION
      ${CMAKE_Fortran_COMPILER_VERSION})
   string( REGEX REPLACE ".* ([0-9]+[.][0-9]+[.-][0-9]+).*" "\\1"
            CMAKE_Fortran_COMPILER_VERSION ${CMAKE_Fortran_COMPILER_VERSION} )
endif()
if( "${CMAKE_Fortran_COMPILER_VERSION}" STRLESS "4.7" )
  message( FATAL_ERROR "
*** Compiler incompatibility:
gfortran < 4.7 will not compile this code.  New versions of gfortran might work but they haven't been tested.  You are trying to use gfortran ${CMAKE_Fortran_COMPILER_VERSION}.
" )
endif()

#
# Compiler Flags
# 

if( NOT Fortran_FLAGS_INITIALIZED )
   set( Fortran_FLAGS_INITIALIZED "yes" CACHE INTERNAL "using draco settings." )

   set( CMAKE_Fortran_FLAGS "-ffree-line-length-none -cpp" )
   set( CMAKE_Fortran_FLAGS_DEBUG 
      "-g -gdwarf-3 -fbounds-check -frange-check -ffpe-trap=invalid,zero,overflow -fbacktrace -finit-integer=2147483647 -finit-real=NAN -finit-character=127 -DDEBUG" )
   set( CMAKE_Fortran_FLAGS_RELEASE "-O3 -mtune=native -ftree-vectorize -funroll-loops -DNDEBUG" )
   set( CMAKE_Fortran_FLAGS_MINSIZEREL "${CMAKE_Fortran_FLAGS_RELEASE}" )
   set( CMAKE_Fortran_FLAGS_RELWITHDEBINFO "-g -gdwarf-3 ${CMAKE_Fortran_FLAGS_RELEASE}")
   
   if (NOT ${CMAKE_GENERATOR} MATCHES Xcode AND HAS_MARCH_NATIVE)
      set( CMAKE_Fortran_FLAGS    "${CMAKE_Fortran_FLAGS} -march=native" )
   endif()
endif()

##---------------------------------------------------------------------------##
# Ensure cache values always match current selection
##---------------------------------------------------------------------------##
set( CMAKE_Fortran_FLAGS                "${CMAKE_Fortran_FLAGS}"                CACHE STRING "compiler flags" FORCE )
set( CMAKE_Fortran_FLAGS_DEBUG          "${CMAKE_Fortran_FLAGS_DEBUG}"          CACHE STRING "compiler flags" FORCE ) 
set( CMAKE_Fortran_FLAGS_RELEASE        "${CMAKE_Fortran_FLAGS_RELEASE}"        CACHE STRING "compiler flags" FORCE )
set( CMAKE_Fortran_FLAGS_MINSIZEREL     "${CMAKE_Fortran_FLAGS_MINSIZEREL}"     CACHE STRING "compiler flags" FORCE )
set( CMAKE_Fortran_FLAGS_RELWITHDEBINFO "${CMAKE_Fortran_FLAGS_RELWITHDEBINFO}" CACHE STRING "compiler flags" FORCE )

#------------------------------------------------------------------------------#
# End config/unix-gfortran.cmake
#------------------------------------------------------------------------------#
