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
	behaviour/belief_data/ThresholdData.cpp
	behaviour/belief_policies/Threshold.cpp
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
#---
	pop/Person.cpp
	pop/PopulationBuilder.cpp
	pop/Population.cpp
#---
	sim/Coordinator.cpp
	sim/Simulator.cpp
	sim/SimulatorBuilder.cpp
	sim/LocalSimulatorAdapter.cpp
	sim/SimulatorRunMode.cpp
	sim/SimulatorSetup.cpp
	util/InstallDirs.cpp
	util/AliasDistribution.cpp
	util/GeoCoordinate.cpp
	util/GeoCoordCalculator.cpp
	util/TravellerScheduleReader.cpp
	util/TransportFacilityReader.cpp
#---
	popgen/PopulationGenerator.cpp
	popgen/utils.cpp
	popgen/FamilyParser.cpp
#---
 	vis/ClusterSaver.cpp
#---
    run/Runner.cpp
)

if ( NOT STRIDE_FORCE_NO_HDF5 )
	list(APPEND LIB_SRC checkpointing/Hdf5Saver.cpp checkpointing/Hdf5Loader.cpp)
endif()

if ( NOT STRIDE_FORCE_NO_MPI )
    list(APPEND LIB_SRC sim/RemoteSimulatorSender.cpp sim/RemoteSimulatorReceiver.cpp)
endif()

set(POPGEN_SRC
#---
	util/InstallDirs.cpp
	util/AliasDistribution.cpp
	util/GeoCoordinate.cpp
	util/GeoCoordCalculator.cpp
#---
	core/ClusterType.cpp
#---
	popgen/PopulationGenerator.cpp
	popgen/utils.cpp
	popgen/FamilyParser.cpp
)

set(MAIN_SRC
	run/main.cpp
)

set(POPGEN_MAIN_SRC
	popgen/main.cpp
)

#============================================================================
# Build & install the executable.
#============================================================================
add_library(libstride ${LIB_SRC})
#target_compile_options(libstride PUBLIC "-flto")
add_executable(stride ${MAIN_SRC} $<TARGET_OBJECTS:trng> )
target_link_libraries(stride libstride ${MPI_LIBRARIES})
#target_compile_options(stride PUBLIC "-flto")

add_library(libpopgen ${POPGEN_SRC})
add_executable(pop_generator ${POPGEN_MAIN_SRC})
target_link_libraries(pop_generator libpopgen ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

target_link_libraries(stride ${LIBS})
if(NOT HDF5_FOUND)
	add_dependencies(stride hdf5_local)
endif()
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
