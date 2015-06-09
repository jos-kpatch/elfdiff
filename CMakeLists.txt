cmake_minimum_required(VERSION 3.2)
project(elfdiff)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories("ELFIO")

set(SOURCE_FILES main.cpp)
add_executable(elfdiff ${SOURCE_FILES})