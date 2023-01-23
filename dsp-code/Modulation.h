#pragma once


#include <algorithm>
#include <cmath>

namespace DSP
{

template <typename T>
class SlowSineLfoOptimized
{
  public:
    // Chamberlin state variable filter
    // only two multiplications and additions per tick
    // don't use values faster than 1/6th of samplerate (e.g. 10000hz @ 48kHz)
    // best suited for low frequencies (e.g. <100hz @ 48kHz)
    explicit SlowSineLfoOptimized(const float sampleRate)
        : m_sampleRate(sampleRate)
    {
    }

    void reset(const T f, const T amplitude = 1.0)
    {
        currentAmplitude = amplitude;
        changeFrequency(f);
        s[0] = amplitude;
        s[1] = 0;
    }

    void changeFrequency(const T f)
    {
        auto w = static_cast<T>(M_PI) * f / m_sampleRate;
        a = 2 * std::sin(w);
    }

    void changeAmplitude(const T amplitude)
    {
        targetAmplitude = amplitude;
        amplitudeChangeSteps = 1024;
        amplitudeAdvance = (amplitude - currentAmplitude) / static_cast<float>(amplitudeChangeSteps);
    }

    void adjustMagnitude(float newMagnitude)
    {
        auto m = currentMagnitude();
        if (m != 0)
        {
            s[0] *= newMagnitude / m;
            s[1] *= newMagnitude / m;
        }
    }

    [[nodiscard]] auto lastSine() const
    {
        return s[1];
    }

    [[nodiscard]] auto lastCosine() const
    {
        return s[0];
    }

    void tick()
    {
        if (amplitudeChangeSteps)
        {
            s[0] *= (currentAmplitude + amplitudeAdvance) / currentAmplitude;
            s[1] *= (currentAmplitude + amplitudeAdvance) / currentAmplitude;
            if (!--amplitudeChangeSteps)
            {
                adjustMagnitude(targetAmplitude);
            }
        }
        s[0] = s[0] - a * s[1];
        s[1] = s[1] + a * s[0];
    }

    float currentMagnitude()
    {
        return std::sqrt(s[0] * s[0] + s[1] * s[1]);
    }

    // only call once per sample
    auto tickSine()
    {
        tick();
        return lastSine();
    }

    // only call once per sample
    auto tickCosine()
    {
        tick();
        return lastCosine();
    }

    void processBlock(T* target, size_t numSamples)
    {
        std::generate_n(target, numSamples, [this]() { return tickSine(); });
    }

  private:
    T m_sampleRate;
    T a{0.00001f};
    T s[2]{0, 0};
    T targetAmplitude{0};
    T currentAmplitude{0};
    T amplitudeAdvance{0.f};
    size_t amplitudeChangeSteps{0};
};


template <typename T>
class SlowSineLfo
{
  public:
    explicit SlowSineLfo(const float sampleRate)
        : m_sampleRate(sampleRate)
    {
    }

    void reset(const T f, const T amplitude = 1.0)
    {
        currentAmplitude = amplitude;
        changeFrequency(f);
    }

    void changeFrequency(const T f)
    {
        m_advance = f / m_sampleRate;
    }

    void changeAmplitude(const T amplitude)
    {
        targetAmplitude = amplitude;
        amplitudeChangeSteps = 1024;
        amplitudeAdvance = (amplitude - currentAmplitude) / static_cast<float>(amplitudeChangeSteps);
    }
    float currentMagnitude()
    {
        return currentAmplitude;
    }
    [[nodiscard]] auto lastSine() const
    {
        return lastValue;
    }

    void tick()
    {
        if (amplitudeChangeSteps)
        {
            currentAmplitude += amplitudeAdvance;
            if (!--amplitudeChangeSteps)
            {
                currentAmplitude = targetAmplitude;
            }
        }
        phase += m_advance;
        if (phase >= 1.0)
        {
            phase -= 1.0;
        }
        lastValue = static_cast<float>(std::sin(phase * M_PI * 2) * currentAmplitude);
    }

    // only call once per sample
    auto tickSine()
    {
        tick();
        return lastSine();
    }

    void processBlock(T* target, size_t numSamples)
    {
        std::generate_n(target, numSamples, [this]() { return tickSine(); });
    }

  private:
    T m_sampleRate;
    double phase{0.f};
    double m_advance{0.00001f};
    T targetAmplitude{0};
    T currentAmplitude{0};
    T amplitudeAdvance{0.f};
    T lastValue;
    size_t amplitudeChangeSteps{0};
};
}
