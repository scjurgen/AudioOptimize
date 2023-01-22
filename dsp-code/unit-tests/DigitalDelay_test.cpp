#include "AudioProcessing.h"
#include "DigitalDelay.h"
#include "ZeroCrossings.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <numbers>


namespace DspTest
{

TEST(DigitalDelayTest, response)
{
    DSP::DigitalDelay<1000> sut{10000.f};
    sut.setModulationSpeed(0.0f);
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

// assume that we do not the phase of the modulation
TEST(DigitalDelayTest, modulation)
{
    DSP::DigitalDelay<2000> sut{48000.f};
    sut.setModulationSpeed(20.f); // 10Hz-> 4800 samples
    sut.setModulationDepth(100.0f);
    // if you set a lower modulation the test should fail (e.g: 98)
    sut.setTime(0.1); // 100msecs at 48000 sr -> ~ 4800 samples delay width
    std::array<float, 48000> target{};

    std::array<float, 48000> feedEmpty{};
    sut.processBlock(feedEmpty.data(), target.data(), target.size());

    std::array<float, 48000> source{};
    {
        std::vector<float> sine(5000, 0);
        DSP::renderSine(sine, 10000.f, 100.f);
        std::copy_n(sine.begin(), sine.size(), source.begin());
    }
    sut.processBlock(source.data(), target.data(), source.size());
    for (size_t i = 0; i < 4700; ++i)
    {
        EXPECT_EQ(target[i], 0);
    }
    auto start = DSP::findFirstZeroCrossingNP(target.data(), target.size());
    EXPECT_LT(start, 5000); // delaytime is 4800, modulation might delay the signal
    size_t minPeriod = 10000, maxPeriod = 0;
    while (start < 9000)
    {
        auto nextZeroCrossing = DSP::findFirstZeroCrossingNP(target.data() + start, target.size() - start - 1);
        minPeriod = std::min(minPeriod, nextZeroCrossing);
        maxPeriod = std::max(maxPeriod, nextZeroCrossing);

        start += nextZeroCrossing;
    }
    EXPECT_LT(minPeriod, 80);
    EXPECT_GT(maxPeriod, 134);
}
}
