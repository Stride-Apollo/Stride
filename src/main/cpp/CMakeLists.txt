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

#============================================================================
# Sources.
#============================================================================
set(LIB_SRC
#---
	calendar/Calendar.cpp
#---
	core/Cluster.cpp
	core/ClusterType.cpp
	core/ContactProfile.cpp
	core/DiseaseProfile.cpp
	core/Health.cpp
	core/Infector.cpp
	core/LogMode.cpp
#---
	output/CasesFile.cpp
	output/PersonFile.cpp
	output/SummaryFile.cpp
#---    
	pop/Person.cpp
	pop/PopulationBuilder.cpp
#---
	sim/run_stride.cpp
	sim/Simulator.cpp
	sim/SimulatorBuilder.cpp
	sim/LocalSimulatorAdapter.cpp
	sim/Coordinator.cpp
#---
	util/InstallDirs.cpp
	util/AliasDistribution.cpp
	util/GeoCoordinate.cpp
	util/GeoCoordCalculator.cpp
#---
	patterns/Publisher.cpp
#---
	hdf5Prototype/subsets.cpp
#---
	popgen/PopulationGenerator.cpp
	popgen/utils.cpp
	popgen/FamilyParser.cpp
)

set(POPGEN_SRC
#---
	util/InstallDirs.cpp
	util/AliasDistribution.cpp
	util/GeoCoordinate.cpp
	util/GeoCoordCalculator.cpp
#---
	popgen/PopulationGenerator.cpp
	popgen/utils.cpp
	popgen/FamilyParser.cpp
)

set(MAIN_SRC
	sim/main.cpp
)

set(POPGEN_MAIN_SRC
	popgen/main.cpp
)

#============================================================================
# HDF5
#============================================================================

# the option is here so that we can plug it into other
# codes where HDF5 may be optional

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
# Build & install the (OpenMP enabled if OpenMP available) executable.
#============================================================================
add_library(libstride ${LIB_SRC})
#target_compile_options(libstride PUBLIC "-flto")
add_executable(stride ${MAIN_SRC} $<TARGET_OBJECTS:trng>)
target_link_libraries(stride libstride)
#target_compile_options(stride PUBLIC "-flto")

add_library(libpopgen ${POPGEN_SRC})
add_executable(pop_generator ${POPGEN_MAIN_SRC})
target_link_libraries(pop_generator libpopgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

target_link_libraries(stride ${LIBS})
if(NOT HDF5_FOUND)
	add_dependencies(stride hdf5_local)
endif()
target_link_libraries(stride ${_hdf5_libs})
#set_target_properties(stride PROPERTIES LINK_FLAGS_RELEASE "-flto")

install(TARGETS stride DESTINATION ${BIN_INSTALL_LOCATION})
install(TARGETS pop_generator DESTINATION ${BIN_INSTALL_LOCATION})

#============================================================================
# Clean up.
#============================================================================
unset(LIB_SRC)
unset(MAIN_SRC)
unset(POPGEN_SRC)
unset(POPGEN_MAIN_SRC)

#############################################################################

