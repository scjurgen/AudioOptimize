
cmake_minimum_required(VERSION 3.21)
project(Documentation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

include_directories(..)
add_executable(BiquadDocumentation Biquad_documentation.cpp)

