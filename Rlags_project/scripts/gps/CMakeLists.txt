cmake_minimum_required (VERSION 2.6)
project (GPS)

find_package(Threads QUIET REQUIRED)

SET(INCLUDE_DIRECTORIES
  "/usr/local/lib/nmealib/include/"
  )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/GPS_Data)

# add the binary tree to the search path for include files
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${INCLUDE_DIRECTORIES}
  )

# add the executable
add_executable(gps getGPS.cpp)
target_link_libraries(gps ${CMAKE_THREAD_LIBS_INIT})

