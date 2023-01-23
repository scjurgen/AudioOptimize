
#include "CrossFader.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace DspTest
{

TEST(CrossFaderTest, rendersCorrectlyEngineersVersion)
{
    DSP::CrossFaderEngineer sut{};
    std::array<float, 1024> fadeIn{};
    std::array<float, 1024> fadeOut{};
    std::array<float, 1024> target{};
    std::fill(fadeIn.begin(), fadeIn.end(), 0.5f);
    std::fill(fadeOut.begin(), fadeOut.end(), 1.f);
    sut.reset(target.size() - 4);
    sut.processBlock(fadeIn.data(), fadeOut.data(), target.data(), target.size());
    EXPECT_GT(target[300], 1.118f);
    EXPECT_GT(target[512], 1.f);
    EXPECT_LT(target[620], 1.f);
    EXPECT_LT(target[678], 0.935f);
    EXPECT_LT(target[791], 0.815f);
    EXPECT_LT(target[904], 0.67f);
    EXPECT_LT(target[1017], 0.505f);
    EXPECT_FLOAT_EQ(target[1023], 0.5f);
}

TEST(CrossFaderTest, rendersCorrectlyPolynomVersion)
{
    DSP::CrossFaderPolynomialApproximation sut{};
    std::array<float, 1024> fadeIn{};
    std::array<float, 1024> fadeOut{};
    std::array<float, 1024> target{};
    std::fill(fadeIn.begin(), fadeIn.end(), 0.5f);
    std::fill(fadeOut.begin(), fadeOut.end(), 1.f);
    sut.reset(target.size() - 4);
    sut.processBlock(fadeIn.data(), fadeOut.data(), target.data(), target.size());
    EXPECT_GT(target[300], 1.118f);
    EXPECT_GT(target[512], 1.f);
    EXPECT_LT(target[620], 1.f);
    EXPECT_LT(target[678], 0.935f);
    EXPECT_LT(target[791], 0.815f);
    EXPECT_LT(target[904], 0.67f);
    EXPECT_LT(target[1017], 0.505f);
    EXPECT_FLOAT_EQ(target[1023], 0.5f);
}

TEST(CrossFaderTest, rendersCorrectlyChamberlinStepper)
{
    DSP::CrossFaderChamberlinStepper sut{};
    std::array<float, 1024> fadeIn{};
    std::array<float, 1024> fadeOut{};
    std::array<float, 1024> target{};
    std::fill(fadeIn.begin(), fadeIn.end(), 0.5f);
    std::fill(fadeOut.begin(), fadeOut.end(), 1.f);
    sut.reset(target.size() - 4);
    sut.processBlock(fadeIn.data(), fadeOut.data(), target.data(), target.size());
    EXPECT_GT(target[300], 1.118f);
    EXPECT_GT(target[512], 1.f);
    EXPECT_LT(target[620], 1.f);
    EXPECT_LT(target[678], 0.935f);
    EXPECT_LT(target[791], 0.815f);
    EXPECT_LT(target[904], 0.67f);
    EXPECT_LT(target[1017], 0.505f);
    EXPECT_FLOAT_EQ(target[1023], 0.5f);
}


TEST(CrossFaderTest, rendersCorrectlyTable)
{
    DSP::CrossFaderTable sut{};
    std::array<float, 1024> fadeIn{};
    std::array<float, 1024> fadeOut{};
    std::array<float, 1024> target{};
    std::fill(fadeIn.begin(), fadeIn.end(), 0.5f);
    std::fill(fadeOut.begin(), fadeOut.end(), 1.f);
    sut.reset(target.size() - 4);
    sut.processBlock(fadeIn.data(), fadeOut.data(), target.data(), target.size());
    EXPECT_GT(target[300], 1.117f);
    EXPECT_GT(target[512], 1.f);
    EXPECT_LT(target[620], 1.f);
    EXPECT_LT(target[678], 0.935f);
    EXPECT_LT(target[791], 0.815f);
    EXPECT_LT(target[904], 0.67f);
    EXPECT_LT(target[1017], 0.505f);
    EXPECT_FLOAT_EQ(target[1023], 0.5f);
}
}
