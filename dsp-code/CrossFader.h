#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>


namespace DSP
{

class CrossFaderEngineer
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
        if (m_isDone)
        {
            return inFade;
        }
        auto fIn = std::sin(m_currentFactorIn * std::numbers::pi_v<float> / 2.f);
        auto fOut = std::sin(m_currentFactorOut * std::numbers::pi_v<float> / 2.f);
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

class CrossFaderTable
{
  public:
    void reset(size_t steps)
    {
        m_steps = steps;
        m_currentFactorIn = 0;
        m_currentFactorOut = steps - 1;
        m_isDone = false;
        if (m_sineTable.size() == steps)
        {
            return;
        }
        m_sineTable.resize(steps);
        for (size_t i = 0; i < steps; ++i)
        {
            m_sineTable[i] = sin(static_cast<float>(i) / static_cast<float>(steps) * std::numbers::pi_v<float> / 2.f);
        }
    }
    std::vector<float> m_sineTable{};
    float step(float inFade, float outFade)
    {
        if (m_isDone)
        {
            return inFade;
        }
        auto fIn = m_sineTable[m_currentFactorIn++];
        auto fOut = m_sineTable[m_currentFactorOut--];
        auto returnValue = inFade * fIn + outFade * fOut;
        if (--m_steps == 0)
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
    size_t m_currentFactorIn{};
    size_t m_currentFactorOut{};

    size_t m_steps{0};
};

class CrossFaderPolynomialApproximation
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
            // it looks like a lot of multiplications but x86_64 does miracles
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
        if (--m_steps == 0)
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

class CrossFaderChamberlinStepper
{
  public:
    void reset(size_t steps)
    {
        m_steps = steps;
        m_isDone = false;
        auto w = std::numbers::pi_v<float> / (steps - 1) / 4.f;
        m_a = 2 * std::sin(w);
        m_s[0] = 1;
        m_s[1] = 0;
        m_s[1] = m_s[1] - m_a * m_s[0];
        m_s[0] = m_s[0] + m_a * m_s[1];
    }

    float step(float inFade, float outFade)
    {
        if (m_isDone)
        {
            return inFade;
        }
        m_s[0] = m_s[0] - m_a * m_s[1];
        m_s[1] = m_s[1] + m_a * m_s[0];
        auto returnValue = inFade * m_s[1] + outFade * m_s[0];
        if (--m_steps == 0)
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
    float m_a{0.00001f};
    float m_s[2]{0, 0};
    bool m_isDone{true};
    size_t m_steps{0};
};

class CrossFaderLinear
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
        if (m_isDone)
        {
            return inFade;
        }
        auto fIn = m_currentFactorIn;
        auto fOut = m_currentFactorOut;
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
