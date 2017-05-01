#############################################################################
#  This is free software: you can redistribute it and/or modify
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
#  Copyright 2017, Willem L, Kuylen E, Stijven S & Broeckhove J
#############################################################################

#============================================================================
# Find HDF5 package on system, if not installed, install localy
#============================================================================

find_package(HDF5)
if(HDF5_FOUND)
    include_directories(${HDF5_INCLUDE_DIR})
    set(_hdf5_libs hdf5 hdf5_cpp)
else()
    # we did not find it so we fetch it from the web, configure, compile, and link
    include(ExternalProject)
    set(ExternalProjectCMakeArgs
        -DHDF5_BUILD_CPP_LIB=ON
        )
    set(_hdf5_version "1.8.15-patch1")
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/hdf5_local/src)
    ExternalProject_Add(hdf5_local
        DOWNLOAD_COMMAND curl http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-${_hdf5_version}.tar.gz | tar xvz -C ${PROJECT_BINARY_DIR}/hdf5_local/src
        CMAKE_ARGS ${ExternalProjectCMakeArgs}
        SOURCE_DIR ${PROJECT_BINARY_DIR}/hdf5_local/src/hdf5-${_hdf5_version}
        BINARY_DIR ${PROJECT_BINARY_DIR}/hdf5_local/build
        STAMP_DIR  ${PROJECT_BINARY_DIR}/hdf5_local/stamp
        TMP_DIR    ${PROJECT_BINARY_DIR}/hdf5_local/tmp
        INSTALL_COMMAND ""
        )
    include_directories(
        ${PROJECT_BINARY_DIR}/hdf5_local/src/hdf5-${_hdf5_version}/src
        ${PROJECT_BINARY_DIR}/hdf5_local/src/hdf5-${_hdf5_version}/c++/src
        ${PROJECT_BINARY_DIR}/hdf5_local/build
        )
    set(_hdf5_libs
        ${PROJECT_BINARY_DIR}/hdf5_local/build/bin/libhdf5_cpp.a
        ${PROJECT_BINARY_DIR}/hdf5_local/build/bin/libhdf5.a
        -ldl
        )
endif()

#============================================================================
# Add subdirectories:
#============================================================================
add_subdirectory( PopGen )

#############################################################################


#============================================================================
# Build & install the (OpenMP enabled if OpenMP available) executable.
#============================================================================
set( EXEC       gtester     )
set( SRC
		main.cpp
		BatchRuns.cpp
		UtilTests.cpp
		PopulationTests.cpp
		LocalSimulatorAdapterTests.cpp
		TravelSchedulerTest.cpp
		TransportFacilityTest.cpp
		PopGen/PopulationGeneratorTest.cpp
		PopGen/FamilyParserTest.cpp
		PopGen/GeoCalculatorTest.cpp
		PopGen/AliasDistributionTest.cpp
		PopGen/RNGPickerTest.cpp
		Hdf5UnitTests.cpp
        Hdf5ScenarioTests.cpp
)

add_executable(${EXEC}   ${SRC} $<TARGET_OBJECTS:trng>)
target_link_libraries(${EXEC} libstride)
target_link_libraries(${EXEC} ${_hdf5_libs})
target_link_libraries( ${EXEC}    ${LIBS} gtest pthread) 
install(TARGETS ${EXEC}  DESTINATION   ${BIN_INSTALL_LOCATION})

#============================================================================
# Define tests.
#============================================================================
add_test( NAME  ${EXEC}_all
	WORKING_DIRECTORY  ${TESTS_DIR}
	COMMAND   ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_LOCATION}/${EXEC}
	--gtest_output=xml:gtester_all.xml
	)

add_test( NAME  ${EXEC}_default
	WORKING_DIRECTORY  ${TESTS_DIR}
	COMMAND   ${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_LOCATION}/${EXEC}
	--gtest_filter=*default*  --gtest_output=xml:gtester_default.xml
	)


#============================================================================
# Clean up.
#============================================================================
unset( EXEC      )
unset( SRC       )

#############################################################################
