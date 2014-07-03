cmake_minimum_required (VERSION 2.6)
project (GPS)

find_package(Threads QUIET REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/GPS_Data)

# add the binary tree to the search path for include files
include_directories(${CMAKE_SOURCE_DIR})

# add the executable
add_executable(gps gps.cpp)
target_link_libraries(gps ${CMAKE_THREAD_LIBS_INIT})
