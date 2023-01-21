#pragma once

#ifdef _WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>

#include "AudioProcessing.h"
#include "OnePoleFilter.h"

namespace DSP
{

class MultiModeFourPoleMixerModule
{
  public:
    explicit MultiModeFourPoleMixerModule(const float sampleRate)
        : lp{OnePoleFilter(sampleRate), OnePoleFilter(sampleRate), OnePoleFilter(sampleRate), OnePoleFilter(sampleRate)}
    {
    }

    void setCutoff(const float cutoff)
    {
        for (auto& f : lp)
        {
            f.setCutoff(cutoff); // cutoff could also be used with different values for different stages, creating very
                                 // different filter characteristics
        }
    }

    void setResonance(const float resonanceNormalized)
    {
        m_resonance = resonanceNormalized * 4.f;
        m_hasResonance = m_resonance != 0.0f;
    }

    void setFactors(const std::array<float, 5>& values)
    {
        m_factors = values;
    }

    void setFactors(const std::array<int, 5>& values)
    {
        std::transform(values.begin(), values.end(), m_factors.begin(), [](int i) { return static_cast<float>(i); });
    }

    float step(float in)
    {
        float feed;

        if (m_hasResonance)
        {
            // auto feed = in - m_stage4 * m_reso;
            // m_v[0] = feed + m_pole * (m_v[0] - feed);
            feed = in - m_stage4 * m_resonance; // resonance could also be fed from the other 4 stages
        }
        else
        {
            feed = in;
        }
        float stage1 = lp[0].next(feed);
        float stage2 = lp[1].next(stage1);
        float stage3 = lp[2].next(stage2);
        m_stage4 = lp[3].next(stage3);
        m_stage4 = std::clamp(m_stage4, -4.f, 4.f);

        float result = feed * m_factors[0];
        result -= stage1 * m_factors[1];
        result += stage2 * m_factors[2];
        result -= stage3 * m_factors[3];
        result += m_stage4 * m_factors[4];
        return result;
    }

    void processBlock(float* inPlace, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            inPlace[i] = step(inPlace[i]);
        }
    }
    void processBlock(const float* in, float* out, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            out[i] = step(in[i]);
        }
    }

    float m_stage4{0};
    float m_resonance{0};
    bool m_hasResonance{false};
    std::array<OnePoleFilter, 4> lp;
    std::array<float, 5> m_factors{0, 0, 0, 0, 1}; // lowpass 24
};


class FourStageFilter
{
  public:
    explicit FourStageFilter(const float sampleRate)
        : FourStageFilter(sampleRate, 1000)
    {
    }

    explicit FourStageFilter(const float sampleRate, const float defaultValue)
        : m_sampleRate(sampleRate)
    {
        setCutoff(defaultValue);
        m_stepsadvance = 0;
        m_pole = m_newpole;
    }

    void setSmoothingSteps(size_t steps)
    {
        m_stepsadvanceSetting = steps;
    }

    void setResonance(float value)
    {
        m_reso = value * 4.0f;
    }

    void setCutoff(const float cutoff)
    {
        m_newpole = std::exp(-2.0f * static_cast<float>(M_PI) * cutoff / m_sampleRate);
        m_stepsadvance = m_stepsadvanceSetting;
        m_advance = (m_newpole - m_pole) / static_cast<float>(m_stepsadvance);
        if (m_stepsadvanceSetting == 0)
        {
            m_pole = m_newpole;
        }
    }

    [[nodiscard]] float currentFactor() const
    {
        return m_pole;
    }

    virtual float singleStep(float in) = 0;

    void processBlock(float* source, size_t numSamples)
    {
        processBlock(source, source, numSamples);
    }

    void processBlock(const float* source, float* target, size_t numSamples)
    {
        size_t index = 0;
        size_t toIndex = numSamples;

        // split into if-less blocks
        if (m_stepsadvance)
        {
            if (m_stepsadvance < numSamples)
            {
                toIndex = m_stepsadvance;
                m_stepsadvance = 0;
            }
            else
            {
                m_stepsadvance -= numSamples;
            }
            while (index < toIndex)
            {
                m_pole += m_advance;
                target[index] = singleStep(source[index]);
                ++index;
            }
            if (!m_stepsadvance)
            {
                m_pole = m_newpole;
            }
        }
        while (index < numSamples)
        {
            target[index] = singleStep(source[index]);
            ++index;
        }
    }

  private:
    float m_sampleRate;
    float m_advance{0};
    size_t m_stepsadvance{0};
    size_t m_stepsadvanceSetting{256u};
    float m_newpole{0.5};

  protected:
    float m_reso{0.0};
    float m_pole{0.5};
    std::array<float, 4> m_v{0, 0, 0, 0};
};


template <std::array<float, 5> f>
class FourStageMultiFilter : public FourStageFilter
{
  public:
    explicit FourStageMultiFilter(const float sampleRate)
        : FourStageFilter(sampleRate, 1000.0)
    {
    }

    float singleStep(const float in) final
    {
        auto feed = in - m_v[3] * m_reso;
        m_v[0] = feed + m_pole * (m_v[0] - feed);
        m_v[1] = m_v[0] + m_pole * (m_v[1] - m_v[0]);
        m_v[2] = m_v[1] + m_pole * (m_v[2] - m_v[1]);
        m_v[3] = m_v[2] + m_pole * (m_v[3] - m_v[2]);
        auto tmpSum = f[0] * feed + f[1] * m_v[0] + f[2] * m_v[1] + f[3] * m_v[2] + f[4] * m_v[3];
        return tmpSum;
    }
};

class VariableFilter1Pole4StageSmooth : public FourStageFilter
{
  public:
    explicit VariableFilter1Pole4StageSmooth(const float sampleRate)
        : FourStageFilter(sampleRate, 1000.0)
    {
    }

    float singleStep(const float in) final
    {
        auto feed = in - m_v[3] * m_reso;
        m_v[0] = feed + m_pole * (m_v[0] - feed);
        m_v[1] = m_v[0] + m_pole * (m_v[1] - m_v[0]);
        m_v[2] = m_v[1] + m_pole * (m_v[2] - m_v[1]);
        m_v[3] = m_v[2] + m_pole * (m_v[3] - m_v[2]);
        auto tmpSum = m_factors[0] * feed + m_factors[1] * m_v[0] + m_factors[2] * m_v[1] + m_factors[3] * m_v[2] +
                      m_factors[4] * m_v[3];
        return tmpSum;
    }

    void setFactors(const std::array<float, 5>& values)
    {
        m_factors = values;
    }

    void setFactors(const std::array<int, 5>& values)
    {
        std::transform(values.begin(), values.end(), m_factors.begin(), [](int i) { return static_cast<float>(i); });
    }

  private:
    std::array<float, 5> m_factors;
};

constexpr static std::array<float, 5> MultiModeLp6 = {0, 1, 0, 0, 0};
constexpr static std::array<float, 5> MultiModeLp12 = {0, 0, 1, 0, 0};
constexpr static std::array<float, 5> MultiModeLp18 = {0, 0, 0, 1, 0};
constexpr static std::array<float, 5> MultiModeLp24 = {0, 0, 0, 0, 1};
constexpr static std::array<float, 5> MultiModeBp6 = {0, -2, 2, 0, 0};
constexpr static std::array<float, 5> MultiModeBp12 = {0, 0, 4, -8, 4};
constexpr static std::array<float, 5> MultiModeHp6 = {1, -1, 0, 0, 0};
constexpr static std::array<float, 5> MultiModeHp12 = {1, -2, 1, 0, 0};
constexpr static std::array<float, 5> MultiModeHp18 = {1, -3, 3, -1, 0};
constexpr static std::array<float, 5> MultiModeHp24 = {1, -4, 6, -4, 1};
constexpr static std::array<float, 5> MultiModePhaser12 = {1, -2, 2, 0, 0};
constexpr static std::array<float, 5> MultiModePhaser24 = {1, -4, 12, -16, 8};
constexpr static std::array<float, 5> MultiModeDoubleNotch = {1, -4, 11, -14, 7};
constexpr static std::array<float, 5> MultiModeNotch12 = {1, -2, 2, 0, 0};
constexpr static std::array<float, 5> MultiModeHp12Lp6 = {0, -3, 6, -3, 0};
constexpr static std::array<float, 5> MultiModeHp18Lp6 = {0, -3, 9, -9, 3};
constexpr static std::array<float, 5> MultiModeNotch12Lp6 = {0, -1, 2, -2, 0};
constexpr static std::array<float, 5> MultiModeAllpass18Lp6 = {0, -1, 3, -6, 4};

// this looks like duplicated, but we use the plain coefficients in the non optimized versions

using Lp6Smooth = FourStageMultiFilter<MultiModeLp6>;
using Lp12Smooth = FourStageMultiFilter<MultiModeLp12>;
using Lp18Smooth = FourStageMultiFilter<MultiModeLp18>;
using Lp24Smooth = FourStageMultiFilter<MultiModeLp24>;

using Bp6Smooth = FourStageMultiFilter<MultiModeBp6>;
using Bp12Smooth = FourStageMultiFilter<MultiModeBp12>;

using Hp6Smooth = FourStageMultiFilter<MultiModeHp6>;
using Hp12Smooth = FourStageMultiFilter<MultiModeHp12>;
using Hp18Smooth = FourStageMultiFilter<MultiModeHp18>;
using Hp24Smooth = FourStageMultiFilter<MultiModeHp24>;

using Phaser12Smooth = FourStageMultiFilter<MultiModePhaser12>;
using Phaser24Smooth = FourStageMultiFilter<MultiModePhaser24>;

using DoubleNotchSmooth = FourStageMultiFilter<MultiModeDoubleNotch>;
using Notch12Smooth = FourStageMultiFilter<MultiModeNotch12>;
using Hp12Lp6Smooth = FourStageMultiFilter<MultiModeHp12Lp6>;
using Hp18Lp6Smooth = FourStageMultiFilter<MultiModeHp18Lp6>;
using Notch12Lp6Smooth = FourStageMultiFilter<MultiModeNotch12Lp6>;
using Allpass18Lp6Smooth = FourStageMultiFilter<MultiModeAllpass18Lp6>;
}