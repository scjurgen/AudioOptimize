#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <numbers>

namespace DSP
{

// render sine wave into buffer, if numChannels>1 the data is interleaved
template <std::floating_point ValueType>
inline void renderSine(std::vector<ValueType>& target, const std::floating_point auto sampleRate,
                       const std::floating_point auto frequency, size_t numChannels = 1)
{
    if (numChannels == 0)
    {
        throw(std::invalid_argument("numChannels can not be 0"));
    }
    class PgenData
    {
      public:
        PgenData(size_t numChannels_, const ValueType a_)
            : numChannels(numChannels_)
            , a(a_)
        {
        }

        ValueType next()
        {
            if (++channel == numChannels)
            {
                channel = 0;
                differential();
            }
            return s[1];
        }

      private:
        const size_t numChannels;
        const ValueType a;
        size_t channel{0};
        std::array<ValueType, 2> s{1.f, 0.f};

        void differential()
        {
            s[0] = s[0] - a * s[1];
            s[1] = s[1] + a * s[0];
        }
    };
    PgenData pGenData{numChannels,
                      static_cast<ValueType>(2 * std::sin(std::numbers::pi_v<ValueType> * frequency / sampleRate))};

    std::generate_n(target.data(), target.size(), [pGenData]() mutable { return pGenData.next(); });
}

template <std::floating_point T>
struct PanValues
{
    T left, right;
};
template <std::floating_point T>
struct MixValues
{
    T dry, wet;
};

template <std::floating_point T>
auto getPanFactor(const T angleNormalized)
{
    if (angleNormalized <= -1.f)
    {
        return PanValues<T>{1.0, 0.0};
    }
    if (angleNormalized >= 1.f)
    {
        return PanValues<T>{0.0, 1.0};
    }
    const std::floating_point auto f = std::sqrt(2.0f) / 2;
    auto angle = angleNormalized * M_PI / 4;
    auto cosVal = std::cos(angle);
    auto sinVal = std::sin(angle);
    return PanValues{static_cast<T>(f * (cosVal - sinVal)), static_cast<T>(f * (cosVal + sinVal))};
}

template <std::floating_point T>
auto getMixFactor(const T mixNormalized)
{
    return getPanFactor(mixNormalized * 2 - 1);
}

template <std::floating_point T>
[[nodiscard]] static auto dbToGain(T dB)
{
    return std::pow(static_cast<T>(10.0), static_cast<T>(dB / 20.0));
}

template <std::floating_point T>
[[nodiscard]] static auto gainToDB(T gain)
{
    if (gain <= 0.0)
    {
        if (std::numeric_limits<T>::is_iec559)
            return -std::numeric_limits<T>::infinity();
        else // 23 bit last bit set.
            return log10(1.0f / static_cast<T>(1 << 23)) * static_cast<T>(20.0);
    }
    return std::log10(gain) * static_cast<T>(20.0);
}
}
