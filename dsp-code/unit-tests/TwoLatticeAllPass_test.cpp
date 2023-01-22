
#include "TwoLatticeAllPass.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace DspTest
{
TEST(TwoLatticeAllPassTest, delayedValues)
{
    std::array<float, 512> source{};
    std::array<float, 512> target{};
    source[0] = 1.f;
    DSP::TwoLatticeAllPass<100> sut{48000.f};
    sut.setSize(70);
    sut.setFeedback(0.5f);
    sut.setLowpassCutoff(24000.f); // disable lowpass
    sut.processBlock(source.data(), target.data(), target.size());
    EXPECT_FLOAT_EQ(target[0], 0.5); // first is just reduced by gain
    EXPECT_FLOAT_EQ(target[70], 0.75);
    EXPECT_FLOAT_EQ(target[140], -0.375);
    EXPECT_FLOAT_EQ(target[210], 0.1875);
    EXPECT_FLOAT_EQ(target[280], -0.09375);
    EXPECT_EQ(target[71], 0);
    EXPECT_EQ(target[141], 0);
    EXPECT_EQ(target[211], 0);
    EXPECT_EQ(target[281], 0);
}

TEST(TwoLatticeAllPassTest, noGain)
{
    std::array<float, 512> source{};
    std::array<float, 512> target{};
    source[0] = 1.f;
    DSP::TwoLatticeAllPass<100> sut{48000.f};
    sut.setSize(70);
    sut.setFeedback(0.0f);
    sut.setLowpassCutoff(24000.f); // disable lowpass
    sut.processBlock(source.data(), target.data(), target.size());
    EXPECT_FLOAT_EQ(target[0], 0.0);
    EXPECT_FLOAT_EQ(target[70], 1.0); // first is delayed
}

TEST(TwoLatticeAllPassTest, unityGain)
{
    std::array<float, 12000> source{};
    std::array<float, 12000> target{};
    source[0] = 1.f;
    DSP::TwoLatticeAllPass<100> sut{48000.f};
    sut.setSize(70);
    sut.setFeedback(1.0f);
    sut.setLowpassCutoff(24000.f); // disable lowpass
    sut.processBlock(source.data(), target.data(), target.size());
    EXPECT_NEAR(target[0], 0.999999f, 1E-7f);
    EXPECT_NE(target[70], 0.0f); // very week signal response
    EXPECT_NE(target[140], 0.0f);
    EXPECT_NE(target[210], 0.0f);
    EXPECT_NE(target[70 * 170], 0.0f);
    EXPECT_FLOAT_EQ(target[70 * 170 - 1], 0.0f);
    EXPECT_FLOAT_EQ(target[70 * 170 + 1], 0.0f);
}


TEST(TwoLatticeAllPassTest, withLowPass)
{
    std::array<float, 512> source{};
    std::array<float, 512> target{};
    source[0] = 1.f;
    DSP::TwoLatticeAllPass<100> sut{48000.f};
    sut.setSize(70);
    sut.setFeedback(0.5f);
    sut.setLowpassCutoff(10000.f);
    sut.processBlock(source.data(), target.data(), target.size());
    EXPECT_FLOAT_EQ(target[0], 0.5); // first is unfiltered and reduced by gain
    EXPECT_GT(target[70], 0.5);
    EXPECT_GT(target[71], 0.1);
    EXPECT_GT(target[72], 0.01);
    EXPECT_LT(target[140], -0.15);
    EXPECT_LT(target[141], -0.1);
    EXPECT_GT(target[210], 0.07);
    EXPECT_GT(target[211], 0.05);
    EXPECT_LT(target[280], -0.02);
    EXPECT_LT(target[281], -0.02);
}
}
