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
set( EXEC       poptester     )
set( SRC
		main.cpp
		GeoCalculatorTest.cpp
		AliasDistributionTest.cpp
		FamilyParserTest.cpp
)

add_executable(${EXEC}   ${SRC} $<TARGET_OBJECTS:trng>)
target_link_libraries(${EXEC} libstride)
target_link_libraries( ${EXEC}    ${LIBS} gtest pthread)
install(TARGETS ${EXEC}  DESTINATION   ${BIN_INSTALL_LOCATION})


#============================================================================
# Include the input files of the tests
#============================================================================
file(COPY "input" DESTINATION "${CMAKE_BINARY_DIR}/test/cpp/gtester/PopGen")

#============================================================================
# Define tests.
#============================================================================
# TODO

#============================================================================
# Clean up.
#============================================================================
unset( EXEC      )
unset( SRC       )

#############################################################################