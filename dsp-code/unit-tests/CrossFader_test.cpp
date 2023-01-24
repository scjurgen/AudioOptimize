
#include "CrossFader.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace DspTest
{
constexpr float epsilon = 1.0E-2f;

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
    EXPECT_NEAR(target[300], 1.118f, epsilon);
    EXPECT_NEAR(target[512], 1.06f, epsilon);
    EXPECT_NEAR(target[620], 0.986f, epsilon);
    EXPECT_NEAR(target[678], 0.935f, epsilon);
    EXPECT_NEAR(target[791], 0.815f, epsilon);
    EXPECT_NEAR(target[904], 0.67f, epsilon);
    EXPECT_NEAR(target[1017], 0.505f, epsilon);
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
    EXPECT_NEAR(target[300], 1.118f, epsilon);
    EXPECT_NEAR(target[512], 1.06f, epsilon);
    EXPECT_NEAR(target[620], 0.986f, epsilon);
    EXPECT_NEAR(target[678], 0.935f, epsilon);
    EXPECT_NEAR(target[791], 0.815f, epsilon);
    EXPECT_NEAR(target[904], 0.67f, epsilon);
    EXPECT_NEAR(target[1017], 0.505f, epsilon);
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
    EXPECT_NEAR(target[300], 1.118f, epsilon);
    EXPECT_NEAR(target[512], 1.06f, epsilon);
    EXPECT_NEAR(target[620], 0.986f, epsilon);
    EXPECT_NEAR(target[678], 0.935f, epsilon);
    EXPECT_NEAR(target[791], 0.815f, epsilon);
    EXPECT_NEAR(target[904], 0.67f, epsilon);
    EXPECT_NEAR(target[1017], 0.505f, epsilon);
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
    EXPECT_NEAR(target[300], 1.117f, epsilon);
    EXPECT_NEAR(target[512], 1.06f, epsilon);
    EXPECT_NEAR(target[620], 0.986f, epsilon);
    EXPECT_NEAR(target[678], 0.935f, epsilon);
    EXPECT_NEAR(target[791], 0.815f, epsilon);
    EXPECT_NEAR(target[904], 0.67f, epsilon);
    EXPECT_NEAR(target[1017], 0.505f, epsilon);
    EXPECT_FLOAT_EQ(target[1023], 0.5f);
}

TEST(CrossFaderTest, rendersVariablePolynomVersion)
{
    DSP::CrossFaderVariablePolynomialApproximation sut{};
    std::array<float, 1024> fadeIn{};
    std::array<float, 1024> fadeOut{};
    std::array<float, 1024> target{};
    std::fill(fadeIn.begin(), fadeIn.end(), 0.5f);
    std::fill(fadeOut.begin(), fadeOut.end(), 1.f);
    sut.reset(target.size() - 4);
    sut.setFactor(0.8);
    sut.processBlock(fadeIn.data(), fadeOut.data(), target.data(), target.size());
    EXPECT_NEAR(target[300], 1.102f, epsilon);
    EXPECT_NEAR(target[512], 1.05f, epsilon);
    EXPECT_NEAR(target[620], 0.986f, epsilon);
    EXPECT_NEAR(target[678], 0.935f, epsilon);
    EXPECT_NEAR(target[791], 0.815f, epsilon);
    EXPECT_NEAR(target[904], 0.67f, epsilon);
    EXPECT_NEAR(target[1017], 0.505f, epsilon);
    EXPECT_FLOAT_EQ(target[1023], 0.5f);
}

TEST(CrossFaderTest, engineerVsTable)
{
    static constexpr auto TableSize = 100u;
    DSP::CrossFaderEngineer eng{};
    DSP::CrossFaderTable tbl{};
    std::array<float, TableSize> fadeIn{};
    std::array<float, TableSize> fadeOut{};
    std::array<float, TableSize> targetEng{};
    std::array<float, TableSize> targetTbl{};
    std::fill(fadeIn.begin(), fadeIn.end(), 0.f);
    std::fill(fadeOut.begin(), fadeOut.end(), 1.f);
    eng.reset(targetEng.size() - 4);
    tbl.reset(targetTbl.size() - 4);
    eng.processBlock(fadeIn.data(), fadeOut.data(), targetEng.data(), targetEng.size());
    tbl.processBlock(fadeIn.data(), fadeOut.data(), targetTbl.data(), targetTbl.size());
    for (size_t i = 0; i < targetEng.size(); ++i)
    {
        EXPECT_FLOAT_EQ(targetEng[i], targetTbl[i]);
    }
}
}
