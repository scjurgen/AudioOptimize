#pragma once

#include <algorithm>
#include <array>
#include <cmath>

namespace DSP
{

class OnePoleFilter
{
  public:
    explicit OnePoleFilter(float sampleRate)
        : m_sampleRate(sampleRate)
    {
    }

    explicit OnePoleFilter(float sampleRate, float defaultCutoff)
        : m_sampleRate(sampleRate)
    {
        setCutoff(defaultCutoff);
    }

    float next(const float in)
    {
        m_v = in + m_fdbk * m_v - m_fdbk * in;
        // m_v = in + m_fdbk * (m_v - in);
        return m_v; // return in-m_v; for highpass
    }

    void processBlock(float* inPlace, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            inPlace[i] = next(inPlace[i]);
        }
    }

    void processBlock(const float* in, float* out, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            out[i] = next(in[i]);
        }
    }

    void setCutoff(const float cutoff)
    {
        if (cutoff >= m_sampleRate / 2)
        {
            m_fdbk = 0;
        }
        else
        {
            auto x = 2.0f * 3.14159265358979f * cutoff / m_sampleRate;
            auto cx = cos(x);
            // m_fdbk = 2 - cx - sqrt((2 - cx) * (2 - cx) - 1);
            m_fdbk = std::exp(-x);
        }
    }

  private:
    float m_sampleRate;
    float m_fdbk{0};
    float m_v{0};
};

class OnePoleFilterOptimized
{
  public:
    explicit OnePoleFilterOptimized(float sampleRate)
        : m_sampleRate(sampleRate)
    {
    }

    explicit OnePoleFilterOptimized(float sampleRate, float defaultCutoff)
        : m_sampleRate(sampleRate)
    {
        setCutoff(defaultCutoff);
    }

    float next(const float in)
    {
        // m_v = in + m_fdbk * m_v - m_fdbk * in;
        m_v = in + m_fdbk * (m_v - in);
        return m_v; // return in-m_v; for highpass
    }

    void processBlock(float* inPlace, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            inPlace[i] = next(inPlace[i]);
        }
    }

    void processBlock(const float* in, float* out, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            out[i] = next(in[i]);
        }
    }

    void setCutoff(const float cutoff)
    {
        if (cutoff >= m_sampleRate / 2)
        {
            m_fdbk = 0;
        }
        else
        {
            m_fdbk = std::exp(-2.0f * 3.14159265358979f * cutoff / m_sampleRate);
        }
    }

  private:
    float m_sampleRate;
    float m_fdbk{0};
    float m_v{0};
};

class SimpleBandpass
{
  public:
    explicit SimpleBandpass(const float sampleRate)
        : m_lp({OnePoleFilter(sampleRate), OnePoleFilter(sampleRate)})
    {
    }

    void setCutoff(const float cutoff)
    {
        // optimizable, but not relevant
        for (auto& lp : m_lp)
        {
            lp.setCutoff(cutoff);
        }
    }

    float next(const float in)
    {
        // optimizable but not necessary if using processBlock?
        auto stage1 = m_lp[0].next(in);
        auto stage2 = m_lp[1].next(stage1);
        return stage2 - stage1;
    }

    void processBlock(float* inPlace, size_t numSamples)
    {
        for (auto& f : m_lp)
        {
            f.processBlock(inPlace, numSamples);
        }
    }

    void processBlock(const float* in, float* out, size_t numSamples)
    {
        m_lp[0].processBlock(in, out, numSamples);
        m_lp[1].processBlock(out, out, numSamples);
    }

  private:
    std::array<OnePoleFilter, 2> m_lp;
};
}