#pragma once

#include "AudioProcessing.h"
#include "TwoLatticeAllPass.h"

#include <array>
#include <vector>

namespace DSP
{

template <size_t MaxDelayLength, size_t NumElements>
class DiffusorDelayChain
{
    template <typename T, size_t N, size_t... Ns, typename... Args>
    auto makeArrayImpl(const std::index_sequence<Ns...>&, Args&&... args)
    {
        const auto create = [&args...](size_t) { return T(args...); };
        return std::array<T, N>{create(Ns)...};
    }

    template <typename T, size_t N, typename... Args>
    auto makeArray(Args&&... args)
    {
        return makeArrayImpl<T, N>(std::make_index_sequence<N>(), std::forward<Args>(args)...);
    }


    explicit DiffusorDelayChain(const float sampleRate)
        : m_sampleRate(sampleRate)
        , m_delay{makeArray<DSP::TwoLatticeAllPass<MaxDelayLength>, NumElements>(sampleRate)}
    {
        for (auto& m : m_delay)
        {
            m.setFeedback(m_feedback);
            m.setLowpassCutoff(8000);
        }
    }

  public:
    [[nodiscard]] auto isPrimeNumber(const unsigned startValue)
    {
        auto n = startValue;
        if (n == 2u || n == 3u)
        {
            return true;
        }
        if (n <= 1u || n % 2u == 0 || n % 3u == 0)
        {
            return false;
        }
        for (auto i = 5u; i * i <= n; i += 6u)
        {
            if (n % i == 0 || n % (i + 2u) == 0)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] auto getUsefulPrime(const unsigned minimumValue, const unsigned wIn) -> decltype(wIn)
    {
        auto n = std::max(minimumValue, wIn) | 1u;
        for (size_t m = 0; m < 250u; ++m)
        {
            if (isPrimeNumber(n))
            {
                return n;
            }
            n += 2u;
        }
        return n;
    }

    void setElementSize(size_t index, size_t widthFor48K)
    {
        auto w = getUsefulPrime(101, static_cast<unsigned>(widthFor48K * 48000.f / m_sampleRate));
        m_delay[index].setSize(w);
    }

    void setCutoff(const float hz)
    {
        for (auto& r : m_delay)
        {
            r.setLowpassCutoff(hz);
        }
    }

    void setMix(float mix)
    {
        m_mix = DSP::getMixFactor(mix);
    }

    void processBlock(const float* source, float* target, const size_t numSamples)
    {
        std::copy_n(source, numSamples, target);
        if (std::fpclassify(m_feedback) == FP_ZERO || std::fpclassify(std::get<1>(m_mix)) == FP_ZERO)
        {
            return;
        }
        for (size_t i = 0; i < NumElements; ++i)
        {
            m_delay[i].processBlockInplace(target, numSamples);
        }
        std::transform(source, source + numSamples, target, target,
                       [m = m_mix](float lhs, float rhs) { return lhs * m.left + rhs * m.right; });
    }

  private:
    float step(float value)
    {
        auto tmp = value;
        for (size_t i = 0; i < NumElements; ++i)
        {
            tmp = m_delay[i].step(tmp);
        }
        return value * m_mix.left + tmp * m_mix.right;
    }
    float m_sampleRate;
    float m_feedback{0.65f};
    std::array<DSP::TwoLatticeAllPass<MaxDelayLength>, NumElements> m_delay;
    std::array<size_t, NumElements> m_decayTimeInSamples{};
    PanValues<float> m_mix{0.7f, 0.7f};
};
}