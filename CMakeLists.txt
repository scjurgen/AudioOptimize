
cmake_minimum_required(VERSION 3.21)
project(AudioOptimize)
add_subdirectory(AudioOptimize)
add_subdirectory(dsp-code/documentation)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(dsp-code/unit-tests)
endif()

