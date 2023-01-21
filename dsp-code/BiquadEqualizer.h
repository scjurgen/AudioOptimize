#pragma once

#include "Biquad.h"

#include <algorithm>
#include <array>
#include <numbers>
#include <vector>

namespace DSP
{

// a simple equalizer with Highpass, Peak and Lowpass
template <size_t MaxOrder>
class BiquadEqualizer
{
  public:
    explicit BiquadEqualizer(float sampleRate)
        : m_sampleRate(sampleRate)
    {
    }

    void setBass(const size_t order, const float hz, const float q)
    {
        m_lowOrder = std::clamp<size_t>(order, 1, MaxOrder);
        m_lowCutoff = hz;
        m_lowQ = q;
        for (auto& f : m_hp)
        {
            f.computeCoefficients(m_sampleRate, m_lowCutoff, m_lowQ, 0.f);
        }
    }

    void setParametric(const float hz, const float q, const float gain)
    {
        m_peakCutoff = hz;
        m_peakQ = q;
        m_peakGain = gain;
        m_usePeak = m_peakGain != 0.0f;
        m_peak.computeCoefficients(m_sampleRate, m_peakCutoff, m_peakQ, m_peakGain);
    }

    void setTreble(const size_t order, const float hz, const float q)
    {
        m_trebleOrder = std::clamp<size_t>(order, 1, MaxOrder);
        m_trebleCutoff = hz;
        m_trebleQ = q;
        for (auto& f : m_lp)
        {
            f.computeCoefficients(m_sampleRate, m_trebleCutoff, m_trebleQ, 0.0f);
        }
    }

    float getMagnitude(const float hz)
    {
        auto hpValue = m_hp[0].magnitudeInDb(hz / m_sampleRate) * m_lowOrder;
        auto peakValue = m_peak.magnitudeInDb(hz / m_sampleRate);
        auto lpValue = m_lp[0].magnitudeInDb(hz / m_sampleRate) * m_trebleOrder;
        auto sum = hpValue + (m_usePeak ? peakValue : 0) + lpValue;
        return static_cast<float>(sum);
    }

    void processBlock(const float* left, const float* right, float* outLeft, float* outRight, size_t numSamples)
    {
        std::copy_n(left, numSamples, outLeft);
        std::copy_n(right, numSamples, outRight);
        for (size_t i = 0; i < m_lowOrder; ++i)
        {
            m_hp[i].processBlock(outLeft, outRight, outLeft, outRight, numSamples);
        }
        if (m_usePeak)
        {
            m_peak.processBlock(outLeft, outRight, outLeft, outRight, numSamples);
        }
        for (size_t i = 0; i < m_trebleOrder; ++i)
        {
            m_lp[i].processBlock(outLeft, outRight, outLeft, outRight, numSamples);
        }
    }

  private:
    float m_sampleRate;

    // don't get confused, the treble pass is for the low frequencies
    BiquadStereo<BiquadFilterType::HighPass> m_hp[MaxOrder]{};
    BiquadStereo<BiquadFilterType::Peak> m_peak{};
    BiquadStereo<BiquadFilterType::LowPass> m_lp[MaxOrder]{};
    size_t m_lowOrder{MaxOrder};
    size_t m_trebleOrder{MaxOrder};
    bool m_usePeak{false};

    float m_lowCutoff{100.f};
    float m_lowQ{1.f / std::numbers::sqrt2_v<float>};
    float m_peakCutoff{500.f};
    float m_peakQ{1.f / std::numbers::sqrt2_v<float>};
    float m_peakGain{0};
    float m_trebleCutoff{500};
    float m_trebleQ{1.f / std::numbers::sqrt2_v<float>};
};


// a simple equalizer with Highpass, Peak and Lowpass
template <size_t MaxOrder>
class BiquadEqualizerSmooth
{
  private:
    struct EqValues
    {
        struct CutoffFilter
        {
            unsigned order;
            float cutoff;
            float qFactor;
        };
        struct PeakFilter
        {
            float cutoff;
            float qFactor;
            float gain;
        };

        CutoffFilter low;
        PeakFilter peak;
        CutoffFilter treble;
    };

  public:
    BiquadEqualizerSmooth(float sampleRate, size_t maxBlockSize)
        : m_eq({BiquadEqualizer<MaxOrder>(sampleRate), BiquadEqualizer<MaxOrder>(sampleRate)})
        , m_tmp({std::vector<float>(maxBlockSize, 0.f), std::vector<float>(maxBlockSize, 0.f),
                 std::vector<float>(maxBlockSize, 0.f), std::vector<float>(maxBlockSize, 0.f)})
    {
    }

    void scheduleSwap()
    {
        if (m_fading)
        {
            m_scheduleSwap = true;
        }
        else
        {
            applyFade();
        }
    }

    void applyFade()
    {
        m_scheduleSwap = false;
        m_newValues = m_scheduledValues;
        m_eq[1 - m_currentEq].setBass(m_newValues.low.order, m_newValues.low.cutoff, m_newValues.low.qFactor);
        m_eq[1 - m_currentEq].setParametric(m_newValues.peak.cutoff, m_newValues.peak.qFactor, m_newValues.peak.gain);
        m_eq[1 - m_currentEq].setTreble(m_newValues.treble.order, m_newValues.treble.cutoff,
                                        m_newValues.treble.qFactor);
        m_fadeStep = 4096;
        m_fadeAdvance = 1.f / static_cast<float>(m_fadeStep);
        m_fadeIn = 0.0f;
        m_fadeOut = 1.0f;
        m_fading = true;
    }

    void setBassOrder(size_t order)
    {
        m_scheduledValues.low.order = std::clamp<size_t>(order, 1, MaxOrder);
        scheduleSwap();
    }

    void setBassCutoff(float hz)
    {
        m_scheduledValues.low.cutoff = std::clamp<float>(hz, 10.f, 24000.0f);
        scheduleSwap();
    }

    void setBassQ(float q)
    {
        m_scheduledValues.low.qFactor = std::clamp<float>(q, 0.001, 20.0);
        scheduleSwap();
    }

    void setParametricCutoff(float hz)
    {
        m_scheduledValues.peak.cutoff = std::clamp<float>(hz, 10.f, 24000.0f);
        scheduleSwap();
    }

    void setParametricQ(float q)
    {
        m_scheduledValues.peak.qFactor = std::clamp<float>(q, 0.001, 20.0);
        scheduleSwap();
    }

    void setParametricGain(float gain)
    {
        m_scheduledValues.peak.gain = std::clamp<float>(gain, -24.f, 24.f);
        scheduleSwap();
    }

    void setTrebleOrder(size_t order)
    {
        m_scheduledValues.treble.order = std::clamp<size_t>(order, 1, MaxOrder);
        scheduleSwap();
    }

    void setTrebleCutoff(float hz)
    {
        m_scheduledValues.treble.cutoff = std::clamp<float>(hz, 10.f, 24000.0f);
        scheduleSwap();
    }

    void setTrebleQ(float q)
    {
        m_scheduledValues.treble.qFactor = std::clamp<float>(q, 0.001, 20.0);
        scheduleSwap();
    }


    float getMagnitude(float hz)
    {
        return m_eq[m_currentEq].getMagnitude(hz);
    }

    void processBlock(const float* left, const float* right, float* outLeft, float* outRight, const size_t numSamples)
    {
        assert(numSamples <= m_tmp[0].size());
        if (m_fading)
        {
            m_eq[0].processBlock(left, right, m_tmp[0].data(), m_tmp[1].data(), numSamples);
            m_eq[1].processBlock(left, right, m_tmp[2].data(), m_tmp[3].data(), numSamples);
            const auto indexFadeIn = (1 - m_currentEq) * 2;
            const auto indexFadeOut = m_currentEq * 2;
            for (size_t i = 0; i < numSamples; ++i)
            {
                if (m_fadeStep)
                {
                    outLeft[i] = m_fadeIn * m_tmp[indexFadeIn][i] + m_fadeOut * m_tmp[indexFadeOut + 1][i];
                    outRight[i] = m_fadeIn * m_tmp[indexFadeIn][i] + m_fadeOut * m_tmp[indexFadeOut + 1][i];
                    --m_fadeStep;
                    m_fadeIn += m_fadeAdvance;
                    m_fadeOut -= m_fadeAdvance;
                    if (m_fadeStep == 0)
                    {
                        m_fading = false;
                        while (++i < numSamples)
                        {
                            outLeft[i] = m_tmp[indexFadeIn][i];
                            outRight[i] = m_tmp[indexFadeIn + 1][i];
                        }
                        // swap eq set, we are done with fade
                        m_currentEq = 1 - m_currentEq;
                        if (m_scheduleSwap)
                        {
                            applyFade();
                        }
                    }
                }
            }
        }
        else
        {
            std::copy_n(left, numSamples, outLeft);
            std::copy_n(right, numSamples, outRight);
            m_eq[m_currentEq].processBlock(outLeft, outRight, outLeft, outRight, numSamples);
        }
    }

  private:
    std::array<std::vector<float>, 4> m_tmp;
    std::array<BiquadEqualizer<MaxOrder>, 2> m_eq{};
    int m_currentEq{0};
    EqValues m_newValues{1, 100.0f, 0.707f, 1000.0f, 0.707f, 0, 1, 8000.f, 0.707f};
    EqValues m_scheduledValues{1, 100.0f, 0.707f, 1000.0f, 0.707f, 0, 1, 8000.f, 0.707f};
    bool m_fading{false};
    float m_fadeIn{0.f};
    float m_fadeOut{0.f};
    float m_fadeAdvance{0.f};
    size_t m_fadeStep{0};
    bool m_scheduleSwap{false};
};

}