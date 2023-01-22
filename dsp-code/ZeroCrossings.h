#pragma once

#include <concepts>
#include <cstddef>

namespace DSP
{

/*
 * find first zero crossing in a data set,
 * return maxSize if none found
 */
template <typename T>
inline size_t findFirstZeroCrossingNP(const T* data, size_t maxSize)
{
    for (size_t index = 1; index < maxSize; ++index)
    {
        if (data[index - 1] < 0 && data[index] >= 0)
        {
            return index;
        }
    }
    return maxSize;
}

/*
 * the type is only for the input vector as it could be int16_t int32_t
 * but the period length is a float
 */
template <std::floating_point T>
inline float periodLengthByZeroCrossingAverage(const T* data, size_t size)
{
    const size_t firstIndex = findFirstZeroCrossingNP(data, size);
    if (firstIndex == size)
    {
        return 0.0f;
    }
    auto previousValue = data[firstIndex];
    size_t cnt = 0;
    size_t lastIndex = firstIndex;
    for (size_t i = firstIndex + 1; i < size; ++i)
    {
        if (previousValue < 0 && (data[i] >= 0))
        {
            ++cnt;
            lastIndex = i;
        }
        previousValue = data[i];
    }
    if (cnt == 0)
    {
        return 0.0f;
    }
    return static_cast<float>(lastIndex - firstIndex) / static_cast<float>(cnt);
}

}
