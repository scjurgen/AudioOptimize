
#include "DigitalDelay.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <numbers>

TEST(DigitalDelayTest, response)
{
    DSP::DigitalDelay<1000> sut{10000.f};
    sut.setModulationSpeed(0.f);
    sut.setModulationDepth(0.0f);
    sut.setTime(0.01); // 10msecs at 10000 sr -> ~ 100 samples delay width
    std::array<float, 1024> feedEmpty{};
    std::array<float, 1024> target{};
    sut.processBlock(feedEmpty.data(), target.data(), target.size());
    sut.processBlock(feedEmpty.data(), target.data(), target.size());
    sut.processBlock(feedEmpty.data(), target.data(), target.size());
    sut.processBlock(feedEmpty.data(), target.data(), target.size());

    std::array<float, 1024> source{};
    source[0] = 1;
    source[1] = -0.5f;
    source[2] = 0.25f;
    source[3] = -0.125f;
    sut.processBlock(source.data(), target.data(), target.size());
    // we are interpolating because of modulation, therefore the signal is a bit 'smeared' inside
    for (size_t i = 0; i < 96; ++i)
    {
        EXPECT_EQ(target[i], 0);
    }
    EXPECT_NE(target[98], 0);
    EXPECT_NE(target[99], 0);
    EXPECT_NE(target[100], 0);
    EXPECT_NE(target[101], 0);
    EXPECT_NE(target[102], 0);
    for (size_t i = 105; i < target.size(); ++i)
    {
        EXPECT_EQ(target[i], 0);
    }
}