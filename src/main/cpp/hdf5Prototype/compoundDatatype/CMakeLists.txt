cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(HDF5prototypeCompound)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

find_package(HDF5 COMPONENTS C HL)
include_directories(${HDF5_INCLUDE_DIRS})
set(_hdf5_libs hdf5 hdf5_cpp)

set(CREATE 
	Createhdf5file.cpp
)
set(LOAD 
	Loadhdf5file.cpp
)
set(UTIL
	Person.cpp
)

set(_hdf5_libs ${_hdf5_libs} ${HDF5_LIBRARIES})

add_executable(CreateData ${CREATE} ${UTIL})
add_executable(LoadData ${LOAD} ${UTIL})
target_link_libraries(CreateData ${_hdf5_libs})
target_link_libraries(LoadData ${_hdf5_libs})

install(TARGETS CreateData DESTINATION ${PROJECT_SOURCE_DIR})
install(TARGETS LoadData DESTINATION ${PROJECT_SOURCE_DIR})