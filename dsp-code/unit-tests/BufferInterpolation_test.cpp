
#include "BufferInterpolation.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace DspTest
{

TEST(DspBufferInterpolationTest, valuesCheck)
{
    std::array<float, 5> a{1, 0, 1, 0, 1};
    auto result = DSP::bspline_43z(a.data(), 0.0);
    EXPECT_FLOAT_EQ(result, 1.f / 3.f);
    result = DSP::bspline_43z(a.data(), 0.25f);
    EXPECT_FLOAT_EQ(result, 0.38541666666f);
    result = DSP::bspline_43z(a.data(), 0.5f);
    EXPECT_FLOAT_EQ(result, 0.5f);
    result = DSP::bspline_43z(a.data(), 0.75f);
    EXPECT_FLOAT_EQ(result, 0.61458331);
    result = DSP::bspline_43z(a.data(), 1.f);
    EXPECT_FLOAT_EQ(result, 2.f / 3.f);
}


TEST(DspBufferInterpolationTest, optimizedValuesCheck)
{
    std::array<float, 5> a{1, 0, 1, 0, 1};
    auto result = DSP::bspline_43x(a.data(), 0.0);
    EXPECT_FLOAT_EQ(result, 1.f / 3.f);
    result = DSP::bspline_43x(a.data(), 0.25f);
    EXPECT_FLOAT_EQ(result, 0.38541666666f);
    result = DSP::bspline_43x(a.data(), 0.5f);
    EXPECT_FLOAT_EQ(result, 0.5f);
    result = DSP::bspline_43x(a.data(), 0.75f);
    EXPECT_FLOAT_EQ(result, 0.61458331);
    result = DSP::bspline_43x(a.data(), 1.f);
    EXPECT_FLOAT_EQ(result, 2.f / 3.f);
}
}
