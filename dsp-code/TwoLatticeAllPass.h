#pragma once
#include "OnePoleFilter.h"

#include <vector>

namespace DSP
{

template <size_t MaxDelayLength>
class TwoLatticeAllPass
{
  public:
    TwoLatticeAllPass(float sampleRate)
        : m_lowpass(sampleRate)
        , m_buffer(MaxDelayLength, 0.f)
    {
        m_buffer.resize(MaxDelayLength);
        clear();
    }

    void clear()
    {
        std::fill(m_buffer.begin(), m_buffer.end(), 0.f);
    }

    void setLowpassCutoff(const float value)
    {
        m_lowpass.setCutoff(value);
    }

    void setFeedback(const float gain)
    {
        m_feedback = std::clamp(gain, -0.999999f, 0.999999f);
    }

    void setSize(const size_t newSize)
    {
        const auto clampedSize = std::clamp<size_t>(newSize, 51, MaxDelayLength);
        m_headRead = MaxDelayLength + m_headWrite - clampedSize;
        if (m_headRead >= MaxDelayLength)
        {
            m_headRead -= MaxDelayLength;
        }
    }

    void processBlock(const float* source, float* target, size_t numSamples)
    {
        std::transform(source, source + numSamples, target, [this](float in) { return step(in); });
    }

    void processBlockInplace(float* inplace, size_t numSamples)
    {
        std::transform(inplace, inplace + numSamples, inplace, [this](float in) { return step(in); });
    }

  private:
    float step(const float in)
    {
        const auto delayedValue = nextHeadRead();
        const auto feedDelay = in - delayedValue * m_feedback; // N.B. negative feedback
        const auto ret = feedDelay * m_feedback + delayedValue;
        m_buffer[m_headWrite++] = m_lowpass.next(feedDelay);
        m_headWrite = m_headWrite % MaxDelayLength;
        return ret;
    }

    float nextHeadRead()
    {
        const auto returnValue = m_buffer[m_headRead];
        m_headRead = (m_headRead + 1) % MaxDelayLength;
        return returnValue;
    }

    OnePoleFilter m_lowpass;
    float m_feedback{0.0f};
    size_t m_headRead{0};
    size_t m_headWrite{0};
    std::vector<float> m_buffer{};
};
}
