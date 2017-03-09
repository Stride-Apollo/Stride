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
#---
	util/InstallDirs.cpp
	util/AliasDistribution.cpp util/AliasDistribution.h
#---
	patterns/Publisher.cpp
)

set(MAIN_SRC
	sim/main.cpp
)

#============================================================================
# Build & install the (OpenMP enabled if OpenMP available) executable.
#============================================================================
add_library(libstride OBJECT ${LIB_SRC})
#target_compile_options(libstride PUBLIC "-flto")
#
add_executable(stride ${MAIN_SRC} $<TARGET_OBJECTS:libstride> $<TARGET_OBJECTS:trng>)
#target_compile_options(stride PUBLIC "-flto")
#
target_link_libraries(stride ${LIBS})
#set_target_properties(stride PROPERTIES LINK_FLAGS_RELEASE "-flto")
install(TARGETS stride DESTINATION ${BIN_INSTALL_LOCATION})

#============================================================================
# Clean up.
#============================================================================
unset(LIB_SRC)
unset(MAIN_SRC)

#############################################################################
