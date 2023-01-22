
#include "gtest/gtest.h"

#include "AudioProcessing.h"
#include "OnePoleFilter.h"

#include <chrono>
namespace DspTest
{
class OnePoleFilterTest : public testing::TestWithParam<std::tuple<double, double>>
{
};

TEST_P(OnePoleFilterTest, checkCorrectMagnitudes)
{
    const float cf = std::get<0>(GetParam());
    const float hz = std::get<1>(GetParam());
    constexpr auto sampleRate{48000.0f};

    DSP::OnePoleFilter sut{sampleRate};
    sut.setCutoff(cf);
    std::vector<float> wave{};
    wave.resize(8000);
    sut.processBlock(wave.data(), wave.data(), wave.size());
    DSP::renderSine(wave, sampleRate, hz);
    sut.processBlock(wave.data(), wave.data(), wave.size());

    const auto [minV, maxV] = std::minmax_element(wave.begin() + wave.size() / 2, wave.end());
    const auto maxValue = std::max(std::abs(*minV), std::abs(*maxV));

    const auto db = std::log10(maxValue) * 20.0f;

    const auto w = hz / cf;
    const auto magnitude = 1 / sqrt(w * w + 1); // Hipass: w/sqrt(w * w + 1)

    const auto expectedDb = std::log10(magnitude) * 20.0;
    if (hz < 3000)
    {
        const auto maxDt = 0.1f;
        EXPECT_NEAR(db, expectedDb, maxDt);
    }
    else // higher frequency is reducing signal
    {
        const auto maxDt = 6.f;
        EXPECT_NEAR(db, expectedDb, maxDt);
    }
}

INSTANTIATE_TEST_SUITE_P(OnePoleMagnitudeTest, OnePoleFilterTest,
                         ::testing::Combine(testing::Values(55.f, 110.f, 220.f, 440.f, 880.f, 1760.f, 3520.f, 7040.f,
                                                            14080.f), // cutoff
                                            testing::Values(25.f, 50.f, 100.f, 200.f, 400.f, 800.f, 1600.f, 3200.f,
                                                            6400.f, 12800.f) // test frequency
                                            ));

TEST(OnePoleFilterTest, testAdaption)
{
    DSP::OnePoleFilter lp(4.0f);
    std::array<float, 128> source{};
    lp.setCutoff(1000);
    lp.processBlock(source.data(), 128);
    EXPECT_FLOAT_EQ(source[0], 0);
}
}
