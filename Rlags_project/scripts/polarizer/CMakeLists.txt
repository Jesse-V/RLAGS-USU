cmake_minimum_required (VERSION 2.6)
project (Polarizer)

# add the binary tree to the search path for include files
include_directories("/usr/local/include/eigen3")
include_directories(${CMAKE_SOURCE_DIR})

# add the executable
add_executable(polarizer polarizerAlan.cpp)
file(COPY ${CMAKE_SOURCE_DIR}/updatePolarizer.sh DESTINATION ${CMAKE_BINARY_DIR}/)
