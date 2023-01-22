#pragma once


#include <cmath>
#include <numbers>


namespace Music
{

template <std::floating_point T>
static auto midiNoteToFrequency(const T note)
{
    constexpr T SemitoneFactor{1.059463094359295f};
    return 440.0f * std::pow(SemitoneFactor, note - 69.0f);
}

}
