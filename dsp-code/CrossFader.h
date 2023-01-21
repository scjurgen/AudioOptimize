#pragma once

#include <algorithm>

namespace DSP
{

class CrossFader
{
  public:
    void reset(size_t steps)
    {
        m_steps = steps;
        m_currentFactorIn = 0;
        m_currentFactorOut = 1.f;
        m_advance = 1.0f / static_cast<float>(steps);
        m_isDone = false;
    }

    float step(float inFade, float outFade)
    {
        auto sineApproximation = [](const float x)
        {
            // this sine approximation is good enough for fading
            return ((0.07278444808570807f * x * x - 0.6434438844902022f) * x * x + 1.570659489151459f) * x;
        };
        if (m_isDone)
        {
            return inFade;
        }
        auto fIn = sineApproximation(m_currentFactorIn);
        auto fOut = sineApproximation(m_currentFactorOut);
        auto returnValue = inFade * fIn + outFade * fOut;

        m_currentFactorOut -= m_advance;
        m_currentFactorIn += m_advance;
        if (m_currentFactorOut <= 0.f)
        {
            m_isDone = true;
        }
        return returnValue;
    }

    void processBlock(const float* fadeIn, const float* fadeOut, float* target, size_t numSamples)
    {
        if (m_isDone)
        {
            std::copy(fadeIn, fadeIn + numSamples, target);
            return;
        }
        for (size_t i = 0; i < numSamples; ++i)
        {
            target[i] = step(fadeIn[i], fadeOut[i]);
        }
    }

    [[nodiscard]] size_t width() const
    {
        return m_steps;
    }

    [[nodiscard]] bool isDone() const
    {
        return m_isDone;
    }

  private:
    bool m_isDone{true};
    float m_currentFactorIn{0.0f};
    float m_currentFactorOut{0.0f};
    float m_advance{0.0f};
    size_t m_steps{0};
};

}