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
# Build & install the (OpenMP enabled if OpenMP available) executable.
#============================================================================
set( EXEC       gtester     )
set( SRC
		main.cpp
		BatchRuns.cpp
		UtilTests.cpp
		PopulationTests.cpp
		# TODO Segfault, pls fix
		#LocalSimulatorAdapterTests.cpp
		# TODO Commented AsyncSimulatorTests, an interface needs no testing
		# Either rewrite it to test the actual class that's responsible
		# For whatever is happening in these tests, or throw them away
		#AsyncSimulatorTests.cpp
		TravelSchedulerTest.cpp
		TransportFacilityTest.cpp
		InfluenceTests.cpp
		DistrictTests.cpp
		Hdf5Base.cpp
		Hdf5UnitTests.cpp
		Hdf5ScenarioTests.cpp
		PopGen/PopulationGeneratorTest.cpp
		PopGen/FamilyParserTest.cpp
		PopGen/GeoCalculatorTest.cpp
		PopGen/AliasDistributionTest.cpp
		Hdf5UnitTests.cpp
		Hdf5ScenarioTests.cpp
		Hdf5Base.cpp
)
if( NOT STRIDE_FORCE_NO_HDF5 )
	set(SRC ${SRC} Hdf5Base.cpp Hdf5UnitTests.cpp Hdf5ScenarioTests.cpp)
endif()

add_executable(${EXEC}   ${SRC} $<TARGET_OBJECTS:trng>)
target_link_libraries(${EXEC} libstride)
target_link_libraries( ${EXEC} ${LIBS} gtest pthread)
install(TARGETS ${EXEC}  DESTINATION   ${BIN_INSTALL_LOCATION})

#============================================================================
# Clean up.
#============================================================================
unset( EXEC      )
unset( SRC       )

#############################################################################
