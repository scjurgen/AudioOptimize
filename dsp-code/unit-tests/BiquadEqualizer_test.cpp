
#include "AudioProcessing.h"
#include "BiquadEqualizer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace DspTest
{

static constexpr double maxDeltaDb = 0.15;

inline void renderWithSineWave(std::vector<float>& target, const double sampleRate, const double frequency)
{
    double phase = 0.0;
    const double advance = frequency / sampleRate;

    for (size_t frameIdx = 0; frameIdx < target.size(); ++frameIdx)
    {
        target[frameIdx] = static_cast<float>(sin(phase * M_PI * 2));
        phase += advance;
        if (phase > 1.0)
        {
            phase -= 1.0;
        }
    }
}


TEST(DspEqualizerTests, magnitudeCorrectOrder1)
{
    constexpr size_t Order{1};
    DSP::BiquadEqualizer<Order> sut(48000.f);
    sut.setBass(Order, 100, 0.907);
    sut.setParametric(1000, 0.407, -4.f);
    sut.setTreble(Order, 6000, 3.707);
    constexpr auto sampleRate{48000.0};

    std::vector<float> wave(48000, 0);
    std::vector<float> left(48000, 0);
    std::vector<float> right(48000, 0);
    for (float hz = 20.f; hz < 15000.f; hz *= 1.2f)
    {
        renderWithSineWave(wave, sampleRate, hz);
        sut.processBlock(wave.data(), wave.data(), left.data(), right.data(), wave.size());

        float expectedDb = sut.getMagnitude(hz);
        {
            const auto [minV, maxV] = std::minmax_element(left.begin() + left.size() / 2, left.end());
            const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
            const auto db = std::log10(maxValue) * 20.0f;

            EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "left channel failure @" << hz;
        }
        {
            const auto [minV, maxV] = std::minmax_element(right.begin() + right.size() / 2, right.end());
            const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
            const auto db = std::log10(maxValue) * 20.0f;
            EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "right channel failure @" << hz;
        }
    }
}

TEST(DspEqualizerTests, magnitudeCorrectOrder2)
{
    constexpr size_t Order{2};
    DSP::BiquadEqualizer<Order> sut(48000.f);
    sut.setBass(Order, 100, 0.907);
    sut.setParametric(1000, 0.407, -4.f);
    sut.setTreble(Order, 6000, 3.707);
    constexpr auto sampleRate{48000.0};

    std::vector<float> wave(48000, 0);
    std::vector<float> left(48000, 0);
    std::vector<float> right(48000, 0);
    for (float hz = 50.f; hz < 10000.f; hz *= 1.2f)
    {
        renderWithSineWave(wave, sampleRate, hz);
        sut.processBlock(wave.data(), wave.data(), left.data(), right.data(), wave.size());

        float expectedDb = sut.getMagnitude(hz);
        {
            const auto [minV, maxV] = std::minmax_element(left.begin() + left.size() / 2, left.end());
            const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
            const auto db = std::log10(maxValue) * 20.0f;

            EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "left channel failure @" << hz;
        }
        {
            const auto [minV, maxV] = std::minmax_element(right.begin() + right.size() / 2, right.end());
            const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
            const auto db = std::log10(maxValue) * 20.0f;
            EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "right channel failure @" << hz;
        }
    }
}


TEST(DspEqualizerTests, magnitudeCorrectOrder4)
{
    constexpr size_t Order{4};
    DSP::BiquadEqualizer<Order> sut(48000.f);
    sut.setBass(Order, 100, 0.907);
    sut.setParametric(1000, 0.407, -4.f);
    sut.setTreble(Order, 6000, 3.707);
    constexpr auto sampleRate{48000.0};

    std::vector<float> wave(48000, 0);
    std::vector<float> left(48000, 0);
    std::vector<float> right(48000, 0);
    // higher frequencies are numerically critical (to low output, essentially noise floor)
    for (float hz = 50.f; hz < 10000.f; hz *= 1.2f)
    {
        renderWithSineWave(wave, sampleRate, hz);
        sut.processBlock(wave.data(), wave.data(), left.data(), right.data(), wave.size());

        float expectedDb = sut.getMagnitude(hz);
        {
            const auto [minV, maxV] = std::minmax_element(left.begin() + left.size() / 2, left.end());
            const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
            const auto db = std::log10(maxValue) * 20.0f;

            EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "left channel failure @" << hz;
        }
        {
            const auto [minV, maxV] = std::minmax_element(right.begin() + right.size() / 2, right.end());
            const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
            const auto db = std::log10(maxValue) * 20.0f;
            EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "right channel failure @" << hz;
        }
    }
}


/*
 * swapping the eq, should cause the power at some cutoff frequency to change
 */
TEST(DspEqualizerTests, swappingEq)
{
    constexpr size_t Order{1};
    DSP::BiquadEqualizerSmooth<Order> sut(48000.f, 48000);
    sut.setBassOrder(Order);
    sut.setBassCutoff(100.f);
    sut.setBassQ(0.707f);
    sut.setParametricCutoff(1000.f);
    sut.setParametricQ(0.707);
    sut.setParametricGain(-4.f);
    sut.setTrebleOrder(Order);
    sut.setTrebleCutoff(6000);
    sut.setTrebleQ(0.707);
    constexpr auto sampleRate{48000.0};

    std::vector<float> wave(48000, 0);
    std::vector<float> left(48000, 0);
    std::vector<float> right(48000, 0);
    // settle current setting
    for (size_t i = 0; i < 10; ++i)
    {
        sut.processBlock(wave.data(), wave.data(), left.data(), right.data(), 2048);
    }

    constexpr auto hz = 100.f;
    renderWithSineWave(wave, sampleRate, hz);
    sut.processBlock(wave.data(), wave.data(), left.data(), right.data(), wave.size());
    float expectedDb = sut.getMagnitude(hz);
    {
        const auto [minV, maxV] = std::minmax_element(left.begin() + left.size() / 2, left.end());
        const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
        const auto db = std::log10(maxValue) * 20.0f;

        EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "left channel failure @" << hz;
    }
    {
        const auto [minV, maxV] = std::minmax_element(right.begin() + right.size() / 2, right.end());
        const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
        const auto db = std::log10(maxValue) * 20.0f;
        EXPECT_NEAR(db, expectedDb, maxDeltaDb) << "right channel failure @" << hz;
    }
    sut.setBassCutoff(500.f);
    renderWithSineWave(wave, sampleRate, hz);
    sut.processBlock(wave.data(), wave.data(), left.data(), right.data(), 5000);

    {
        const auto [minV, maxV] = std::minmax_element(left.begin() + 200, left.begin() + 300);
        const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
        const auto db = std::log10(maxValue) * 20.0f;

        EXPECT_LT(db, expectedDb) << "left channel failure @" << hz;
    }
    {
        const auto [minV, maxV] = std::minmax_element(right.begin() + +200, right.begin() + 300);
        const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));
        const auto db = std::log10(maxValue) * 20.0f;
        EXPECT_LT(db, expectedDb) << "right channel failure @" << hz;
    }
}
}
