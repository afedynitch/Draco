#-----------------------------*-cmake-*----------------------------------------#
# file   VendorsChecks/CMakeLists.txt
# author Kelly Thompson <kgt@lanl.gov>
# date   2016 May 11
# brief  Small test of Vendors libraries (if found)
# note   Copyright (C) 2016-2019, Triad National Security, LLC.
#        All rights reserved.
#------------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.9.0)
project( VendorsChecks CXX )

# This component only has tests.

if( BUILD_TESTING )
  add_subdirectory(test)
endif()

#------------------------------------------------------------------------------#
# end VendorsChecks/CMakeLists.txt
#------------------------------------------------------------------------------#