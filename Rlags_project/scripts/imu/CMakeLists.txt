cmake_minimum_required (VERSION 2.6)
project (IMU)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lm")
# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/RLAGS_Data)

# add the binary tree to the search path for include files
# include_directories(${CMAKE_SOURCE_DIR})
link_libraries(m)

# add the executable
add_executable(d2 imu_d2.c)
add_executable(cc imu_cc.c)
# target_link_libraries(serial ${CMAKE_THREAD_LIBS_INIT})
file(COPY ${CMAKE_SOURCE_DIR}/move_imu_data.sh ${CMAKE_SOURCE_DIR}/get_imu_data.sh DESTINATION ${CMAKE_BINARY_DIR}/)
