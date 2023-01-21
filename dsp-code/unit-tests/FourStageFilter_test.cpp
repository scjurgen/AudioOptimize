#include "gtest/gtest.h"

#include "FourStageFilter.h"
#include <random>


#include <algorithm>
#include <array>
#include <cmath>

// this class calculates the magnitude and phase response of the filter as theoretical values
// the math for it is found here: https://expeditionelectronics.com/Diy/Polemixing/math

class MultifilterTheoretical
{
  public:
    double x0, x1, x2, x3, x4;

    MultifilterTheoretical(double a, double b, double c, double d, double e)
    {
        x4 = a;
        x3 = 4 * a + b;
        x2 = 6 * a + 3 * b + c;
        x1 = 4 * a + 3 * b + 2 * c + d;
        x0 = a + b + c + d + e;
    }

    double magnitude(double w, double resonance)
    {
        auto nReal = x0 + x4 * w * w * w * w - x2 * w * w;
        auto nImg = -x1 * w + x3 * w * w * w;
        auto dReal = 1.0 - 6.0 * w * w + w * w * w * w + resonance;
        auto dImg = -4.0 * w + 4 * w * w * w;
        return sqrt(nReal * nReal + nImg * nImg) / sqrt(dReal * dReal + dImg * dImg);
    }

    void magnitudeAndPhase(double w, double resonance, double& magnitude, double& phase)
    {
        // |G| = |N| / |D|
        // P = Pn - Pd
        auto nReal = x0 + x4 * w * w * w * w - x2 * w * w;
        auto nImg = -x1 * w + x3 * w * w * w;
        auto nMag = sqrt(nReal * nReal + nImg * nImg);
        auto nPhase = calcPhase(nImg, nReal);

        auto dReal = 1.0 - 6.0 * w * w + w * w * w * w + resonance;
        auto dImg = -4.0 * w + 4 * w * w * w;
        auto dMag = sqrt(dReal * dReal + dImg * dImg);
        auto dPhase = calcPhase(dImg, dReal);

        magnitude = nMag / dMag;

        auto phaseValue = nPhase - dPhase;
        while (phaseValue > M_PI)
        {
            phaseValue -= 2 * M_PI;
        }
        while (phaseValue < -M_PI)
        {
            phaseValue += 2 * M_PI;
        }
        phase = phaseValue;
    }

    double calcPhase(double img, double real)
    {
        return real > 0 ? -atan(img / real) : -(M_PI - std::atan(img / std::abs(real)));
    }
};


std::array<std::array<float, 5>, 10> filterTestSet{{{0, -1, 0, 0, 0},
                                                    {0, 0, 1, 0, 0},
                                                    {0, 0, 0, -1, 0},
                                                    {0, 0, 0, 0, 1},
                                                    {0, -2, 2, 0, 0},
                                                    {0, 0, 4, -8, 4},
                                                    {1, -1, 0, 0, 0},
                                                    {1, -2, 1, 0, 0},
                                                    {1, -3, 3, -1, 0},
                                                    {1, -4, 6, -4, 1}}};


class Filter4TheoreticalValues : public testing::TestWithParam<std::tuple<std::array<float, 5>, double, double, double>>
{
};
/*
 * check if the measured magnitudes by sending a pure sine through the filter
 * match the theoretical values of the filter
 * resonance is a b**** so expect higher variations
 *
 */
TEST_P(Filter4TheoreticalValues, allMagnitudes)
{
    const std::array<float, 5> ts(std::get<0>(GetParam()));
    const double cf = std::get<1>(GetParam());
    const double hz = std::get<2>(GetParam());
    const double reso = std::get<3>(GetParam());
    constexpr double sampleRate{48000};

    DSP::VariableFilter1Pole4StageSmooth sut(sampleRate);
    sut.setFactors(ts);
    sut.setCutoff(cf);
    sut.setResonance(reso);
    std::vector<float> wave{};
    wave.resize(24000);
    sut.processBlock(wave.data(), wave.data(), wave.size());
    DSP::renderSine(wave, sampleRate, hz);
    sut.processBlock(wave.data(), wave.data(), wave.size());
    auto maxValue = 0.0;
    // pick values from within to account for smoothing and settling
    for (size_t i = wave.size() / 2; i < wave.size(); ++i)
    {
        if (std::abs(wave[i]) > maxValue)
        {
            maxValue = std::abs(wave[i]);
        }
    }
    auto db = std::log10(maxValue) * 20.0;

    MultifilterTheoretical ft{ts[0], ts[1], ts[2], ts[3], ts[4]};
    auto magnitude = ft.magnitude(hz / cf, reso * 4.0);
    auto expectedDb = std::log10(magnitude) * 20.0;
    auto minDt = -3;
    auto maxDt = 3;
    EXPECT_GE(db, expectedDb + minDt) << "Filter out of expected range! "
                                      << "\thz:" << hz << " \tcutoff:" << cf << "\treso:" << reso << "\tmeasured:" << db
                                      << "\texpected:" << expectedDb;
    EXPECT_LE(db, expectedDb + maxDt) << "Filter out of expected range! "
                                      << "\thz:" << hz << " \tcutoff:" << cf << "\treso:" << reso << "\tmeasured:" << db
                                      << "\texpected:" << expectedDb;
    ;
}


INSTANTIATE_TEST_SUITE_P(DSPFilter4StageTest, Filter4TheoreticalValues,
                         ::testing::Combine(testing::ValuesIn(filterTestSet), testing::Values(220.0, 440.0, 880.0),
                                            testing::Values(100.0, 200.0, 400.0, 800.0, 1600.0, 3200.0),
                                            testing::Values(0.0, 0.20, 0.50)));
