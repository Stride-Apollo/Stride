#############################################################################
#  This file is part of the Stride software. 
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or any 
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Copyright 2016, Willem L, Kuylen E & Broeckhove J
#############################################################################
#
#  CMake config.
#
#############################################################################
cmake_minimum_required(VERSION 2.8)

#============================================================================
# Macros that need to be set prior to project definition:
#============================================================================
set( CMAKE_BUILD_TYPE "Release"
	CACHE STRING "Build type: None Debug Release RelWithDebInfo MinSizeRel." )
#
set( CMAKE_INSTALL_PREFIX  "${CMAKE_BINARY_DIR}/installed" 
	CACHE PATH "Prefix prepended to install directories" )
#
set( CMAKE_PROGRAM_PATH  "/opt/local/bin;/usr/texbin;/usr/local/bin;/usr/bin" 
	CACHE PATH "Where to look with find_program." )
	
#============================================================================
# Project name, configuration for CMake, macros for build & install.
#============================================================================
project( STRIDE )
include( CMakeCMakeConfig.txt )
include( CMakeBuildConfig.txt )

#============================================================================
# Configuration for CPP.
#============================================================================
include( CMakeCPPConfig.txt    )
  	
#============================================================================
# Configuration for Git.
#============================================================================
find_package( Git QUIET)

if( GIT_FOUND )
	GIT_WC_INFO(${CMAKE_SOURCE_DIR}/.. STRIDE)
else()
    # This is done to eliminate blank output of undefined CMake variables.
    set( GIT_FOUND FALSE ) 
	set( STRIDE_WC_REVISION_HASH    "0000-0000-000" )	
	set( STRIDE_WC_LAST_CHANGED_DATE   "0000 0000 0000" )
endif()

#============================================================================
# Configuration for DOC: DOXYGEN AND LATEX
#============================================================================
if( STRIDE_INCLUDE_DOC )
    find_package( Doxygen )	
    find_package( LATEX )
    if( NOT "${PDFLATEX_COMPILER}" STREQUAL "PDFLATEX_COMPILER-NOTFOUND" )
    	set( LATEX_FOUND YES )
    endif()
endif()
#============================================================================
# Add subdirs
#============================================================================
add_subdirectory( main )
add_subdirectory( test )
if( STRIDE_INCLUDE_DOC )
	add_subdirectory( doc )
endif()

#============================================================================
# Overview report:
#============================================================================
include( CMakeConfigReport.txt )

#############################################################################
