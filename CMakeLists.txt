cmake_minimum_required(VERSION 3.5)
project(simon_speck)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Speck.cpp)
add_executable(simon_speck ${SOURCE_FILES})