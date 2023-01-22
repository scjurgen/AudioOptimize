#include "Modulation.h"

#include "gtest/gtest.h"

#include <array>
#include <iostream>

namespace DspTest
{
TEST(ModulationTest, slowLfo)
{
    DSP::SlowSineLfo<float> sut{4800.f};
    sut.reset(1.0f, 1.0f);
    sut.tick();
    for (size_t c = 0; c < 1000; ++c)
    {
        for (size_t i = 0; i < 4797; ++i)
        {
            sut.tick();
        }
        auto a = sut.tickSine();
        sut.tick();
        auto b = sut.tickSine();
        EXPECT_LT(a, 0) << "a fails at " << c;
        EXPECT_GT(b, 0) << "b fails at " << c;
    }
}

TEST(ModulationTest, slowLfoLongTermDrift)
{
    auto onePeriod = [](DSP::SlowSineLfo<float>& lfo)
    {
        size_t count = 0;
        do
        {
            ++count;
        } while (lfo.tickSine() >= 0);
        do
        {
            ++count;
        } while (lfo.tickSine() < 0);
        return count;
    };

    auto realLongTimeFrequency = [&onePeriod](const float sampleRate, const float idealFrequency)
    {
#ifdef NDEBUG
        constexpr size_t maxSamples = 48000 * 60 * 10; // 10 minutes at 48000kHz
#else
        constexpr size_t maxSamples = 48000 * 30; // 30 seconds only, save time in debug
#endif
        size_t totalSamplesCount = 0;
        size_t periodsCount = 0;
        DSP::SlowSineLfo<float> lfo(sampleRate);
        lfo.reset(idealFrequency);
        onePeriod(lfo);
        do
        {
            totalSamplesCount += onePeriod(lfo);
            periodsCount++;
        } while (totalSamplesCount < maxSamples);
        return static_cast<float>(totalSamplesCount) / static_cast<float>(periodsCount);
    };
    // very strict tolerance
    constexpr auto epsilon = 1.E-16f;
    EXPECT_NEAR(realLongTimeFrequency(48000.f, 8000.f), 6.f, epsilon);
    EXPECT_NEAR(realLongTimeFrequency(48000.f, 7777.f), 6.172045776f, epsilon);
    EXPECT_NEAR(realLongTimeFrequency(48000.f, 1000.f), 48.f, epsilon);
    EXPECT_NEAR(realLongTimeFrequency(48000.f, 100.f), 480.f, epsilon);
    EXPECT_NEAR(realLongTimeFrequency(48000.f, 10.f), 4800.f, epsilon);
#ifdef NDEBUG
    EXPECT_NEAR(realLongTimeFrequency(48000.f, .5f), 96000.f, 1.f);
    EXPECT_NEAR(realLongTimeFrequency(48000.f, .2f), 240000.f, 1.f);
    // very slow lfo will have a tiny error (1 sample in 10 minutes!)
#endif
}

TEST(ModulationTest, speedAdjust)
{
    auto onePeriod = [](DSP::SlowSineLfo<float>& lfo)
    {
        size_t count = 0;
        do
        {
            ++count;
        } while (lfo.tickSine() >= 0);
        do
        {
            ++count;
        } while (lfo.tickSine() < 0);
        return count;
    };

    size_t totalSamplesCount = 0;
    size_t periodsCount = 0;
    DSP::SlowSineLfo<float> lfo(48000.f);
    lfo.reset(10.f);
    onePeriod(lfo);
    EXPECT_FLOAT_EQ(onePeriod(lfo), 4800);
    lfo.changeFrequency(20.f);
    onePeriod(lfo);
    EXPECT_FLOAT_EQ(onePeriod(lfo), 2400);
    lfo.changeFrequency(7.f);
    onePeriod(lfo);
    EXPECT_FLOAT_EQ(onePeriod(lfo), 6857);
}

TEST(ModulationTest, amplitudeAdjust)
{
    DSP::SlowSineLfo<float> sut(48000.f);
    sut.reset(100.f, 1.0f);
    for (size_t i = 0; i < 100; ++i)
    {
        sut.tick();
        EXPECT_NEAR(sut.currentMagnitude(), 1.f, 0.01);
    }
    sut.changeAmplitude(22.f);

    for (size_t i = 0; i < 1024; ++i)
    {
        sut.tick();
    }
    EXPECT_NEAR(sut.currentMagnitude(), 22, 0.01);
    sut.changeAmplitude(10.f);
    for (size_t i = 0; i < 1024; ++i)
    {
        sut.tick();
    }
    EXPECT_NEAR(sut.currentMagnitude(), 10, 0.01);
    sut.changeAmplitude(0.2f);
    for (size_t i = 0; i < 1024; ++i)
    {
        sut.tick();
    }
    EXPECT_NEAR(sut.currentMagnitude(), 0.2, 0.01);
}
}
