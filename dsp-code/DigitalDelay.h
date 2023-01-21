#pragma once

#include "BufferInterpolation.h"
#include "Modulation.h"

#include <array>
#include <vector>


namespace DSP
{

// a simple delay with modulation (no feedback, no taps)
template <size_t TimeInMilliseconds>
class DigitalDelay
{
  public:
    static constexpr size_t MaxInterpolationOrder = 5;
    explicit DigitalDelay(const float sampleRate)
        : m_sampleRate(sampleRate)
        , m_bufferSize(static_cast<size_t>(sampleRate * static_cast<float>(TimeInMilliseconds) / 1000.f))
        , m_buffer(m_bufferSize + MaxInterpolationOrder)
        , m_delayTime(sampleRate / 4) // 250 msecs default
        , m_modulation(sampleRate)
    {
    }

    void setTime(const float seconds)
    {
        if (m_fadeInOut == 0)
        {
            m_newDelayTime = static_cast<size_t>(ceil(seconds * m_sampleRate));
            m_newDelayTime = std::min(static_cast<size_t>(m_newDelayTime), m_bufferSize - 1000);
            m_fadeInOut = 8192;
            m_fadeIn = 0.0f;
            m_fadeOut = 1.0f;
            m_fadeAdvance = 1.f / static_cast<float>(m_fadeInOut);
        }
        else
        {
            m_newDelayTimeScheduled = seconds;
        }
    }

    void setModulationDepth(const float value)
    {
        m_modulation.changeAmplitude(value);
    }

    void setModulationSpeed(const float valueInHz)
    {
        m_modulation.changeFrequency(valueInHz);
    }

    float readDelayValue(float delayTime)
    {
        auto mpos = m_head - m_modulation.tickSine() - delayTime;
        if (mpos < 0)
        {
            mpos += m_bufferSize;
        }
        float readPos;
        auto fraction = std::modf(mpos, &readPos);
        return bspline_43z(&m_buffer[static_cast<size_t>(readPos)], fraction);
    }

    float step(float inValue)
    {
        auto fadeValue = [this]()
        {
            auto result = m_fadeOut * readDelayValue(m_delayTime);
            result += m_fadeIn * readDelayValue(m_newDelayTime);
            m_fadeOut -= m_fadeAdvance;
            m_fadeIn += m_fadeAdvance;
            m_fadeInOut--;
            if (!m_fadeInOut)
            {
                m_delayTime = m_newDelayTime;
                if (m_newDelayTimeScheduled != 0.f)
                {
                    setTime(m_newDelayTimeScheduled);
                    m_newDelayTimeScheduled = 0.f;
                }
            }
            return result;
        };
        auto result = m_fadeInOut ? fadeValue() : readDelayValue(m_delayTime);

        m_buffer[m_head] = inValue;
        if (m_head < MaxInterpolationOrder)
        {
            m_buffer[m_head + m_bufferSize] = inValue;
        }
        m_head = ++m_head % m_bufferSize;
        return result;
    }

    void processBlock(const float* in, float* out, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            out[i] = step(in[i]);
        }
    }

    float m_sampleRate;
    size_t m_bufferSize;
    std::vector<float> m_buffer;

    size_t m_head{0};
    float m_delayTime{0.f};
    float m_newDelayTime{0.f};
    float m_newDelayTimeScheduled{0.f};
    size_t m_fadeInOut{0};
    float m_fadeIn{0.f};
    float m_fadeOut{1.f};
    float m_fadeAdvance{0.f};
    DSP::SlowSineLfo<float> m_modulation;
};


// a simple delay with modulation (no feedback, no taps)
template <size_t TimeInMilliseconds>
class DigitalDelayOptimized
{
  public:
    static constexpr size_t MaxInterpolationOrder = 5;
    explicit DigitalDelayOptimized(const float sampleRate)
        : m_sampleRate(sampleRate)
        , m_bufferSize(static_cast<size_t>(sampleRate * static_cast<float>(TimeInMilliseconds) / 1000.f))
        , m_buffer(m_bufferSize + MaxInterpolationOrder)
        , m_delayTime(sampleRate / 4) // 250 msecs default
        , m_modulation(sampleRate)
    {
    }

    void setTime(const float seconds)
    {
        if (m_fadeInOut == 0)
        {
            m_newDelayTime = static_cast<size_t>(ceil(seconds * m_sampleRate));
            m_newDelayTime = std::min(static_cast<size_t>(m_newDelayTime), m_bufferSize - 1000);
            m_fadeInOut = 8192;
            m_fadeIn = 0.0f;
            m_fadeOut = 1.0f;
            m_fadeAdvance = 1.f / static_cast<float>(m_fadeInOut);
        }
        else
        {
            m_newDelayTimeScheduled = seconds;
        }
    }

    void setModulationDepth(const float value)
    {
        m_modulation.changeAmplitude(value);
    }

    void setModulationSpeed(const float valueInHz)
    {
        m_modulation.changeFrequency(valueInHz);
    }

    float readDelayValue(float delayTime)
    {
        auto mpos = m_head - m_modulation.tickSine() - delayTime;
        if (mpos < 0)
        {
            mpos += m_bufferSize;
        }
        float readPos;
        auto fraction = std::modf(mpos, &readPos);
        return bspline_43z(&m_buffer[static_cast<size_t>(readPos)], fraction);
    }

    float read()
    {
        auto fadeValue = [this]()
        {
            auto result = m_fadeOut * readDelayValue(m_delayTime);
            result += m_fadeIn * readDelayValue(m_newDelayTime);
            m_fadeOut -= m_fadeAdvance;
            m_fadeIn += m_fadeAdvance;
            m_fadeInOut--;
            if (!m_fadeInOut)
            {
                m_delayTime = m_newDelayTime;
                if (m_newDelayTimeScheduled != 0.f)
                {
                    setTime(m_newDelayTimeScheduled);
                    m_newDelayTimeScheduled = 0.f;
                }
            }
            return result;
        };
        return m_fadeInOut ? fadeValue() : readDelayValue(m_delayTime);
    }

    float step(float inValue)
    {
        auto result = read();
        m_buffer[m_head] = inValue;
        if (m_head < MaxInterpolationOrder)
        {
            m_buffer[m_head + m_bufferSize] = inValue;
        }
        m_head = ++m_head % m_bufferSize;
        return result;
    }

    float stepNoIf(float inValue)
    {
        auto result = read();
        m_buffer[m_head++] = inValue;
        return result;
    }

    void processBlock(const float* in, float* out, size_t numSamples)
    {
        if (m_head < m_bufferSize - numSamples && m_head >= MaxInterpolationOrder)
        {
            for (size_t i = 0; i < numSamples; ++i)
            {
                out[i] = stepNoIf(in[i]);
            }
        }
        else
        {
            for (size_t i = 0; i < numSamples; ++i)
            {
                out[i] = step(in[i]);
            }
        }
    }

    float m_sampleRate;
    size_t m_bufferSize;
    std::vector<float> m_buffer;

    size_t m_head{0};
    float m_delayTime{0.f};
    float m_newDelayTime{0.f};
    float m_newDelayTimeScheduled{0.f};
    size_t m_fadeInOut{0};
    float m_fadeIn{0.f};
    float m_fadeOut{1.f};
    float m_fadeAdvance{0.f};
    DSP::SlowSineLfo<float> m_modulation;
};
}