cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    gtest_discover_tests(${TESTNAME}
            WORKING_DIRECTORY ${PROJECT_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
            )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

add_subdirectory("${PROJECT_SOURCE_DIR}/3rdparty/googletest" "googletest")
mark_as_advanced(
        BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
        gmock_build_tests gtest_build_samples gtest_build_tests
        gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

include_directories(../)
package_add_test(DspCodeGenerators_test
        BeatsList_test.cpp
        )

package_add_test(DspCode_test
  Biquad_test.cpp
  BufferInterpolation_test.cpp
  CrossFader_test.cpp
  DigitalDelay_test.cpp
  BiquadEqualizer_test.cpp
  FourStageFilter_test.cpp
  Modulation_test.cpp
  OnePoleFilter_test.cpp
  TwoLatticeAllPass_test.cpp
  )

package_add_test(DspCodePerformance_test
  Biquad_test.cpp
  CrossFader_test.cpp
  DigitalDelay_test.cpp
  FourStageFilter_test.cpp
  Modulation_test.cpp
  OnePoleFilter_test.cpp
  TwoLatticeAllPass_test.cpp

  performance/BiquadPerformance_test.cpp
  performance/BufferInterpolationPerformance_test.cpp
  performance/CrossFaderPerformance_test.cpp
  performance/DigitalDelayPerformance_test.cpp
  performance/FourStageFilterPerformance_test.cpp
  performance/ModulationPerformance_test.cpp
  performance/OnePoleFilterPerformance_test.cpp
  performance/PerformanceTest_test.cpp
  performance/TwoLatticeAllPassPerformance_test.cpp
  )
