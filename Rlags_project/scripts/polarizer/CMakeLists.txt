cmake_minimum_required (VERSION 2.6)
project (Polarizer)

# add the binary tree to the search path for include files
# so that we will find PolarizerConfig.h
include_directories("/usr/local/include/eigen3")
 
# add the executable
add_executable(polarizer polarizer.cpp)