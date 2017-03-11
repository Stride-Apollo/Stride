#============================================================================
# Sources.
#============================================================================
set(LIB_SRC
        PopulationGenerator.cpp PopulationGenerator.h)

set(MAIN_SRC
        main.cpp)

#============================================================================
# Build & install the (OpenMP enabled if OpenMP available) executable.
#============================================================================
add_library(libpopgen ${LIB_SRC})
#target_compile_options(libstride PUBLIC "-flto")
#
add_executable(popgen ${MAIN_SRC})
target_link_libraries(popgen libpopgen)
#target_compile_options(stride PUBLIC "-flto")
#
target_link_libraries(popgen ${LIBS})
#set_target_properties(stride PROPERTIES LINK_FLAGS_RELEASE "-flto")
install(TARGETS popgen DESTINATION ${BIN_INSTALL_LOCATION})

#============================================================================
# Clean up.
#============================================================================
unset(LIB_SRC)
unset(MAIN_SRC)

#############################################################################
