#pragma once

#include "AudioProcessing.h"
#include "BeatsList.h"
#include "DiffusorDelayChain.h"
#include "DigitalDelay.h"
#include "FourStageFilter.h"

#include <iostream>

using namespace std::string_literals;

template <size_t maxDelayTimeInMilliseconds>
class KindOfADelay
{
    static constexpr size_t InternalBlockSize = 16;
    const std::array<BusinessLogic::BeatsItem, 59> m_beatsList{{
        {"1/64 triplet"s, "1/96"s, 0.0416666679084301},   // 0
        {"1/64"s, ""s, 0.0625},                           // 1
        {"1/32 triplet"s, "1/48"s, 0.0833333358168602},   // 2
        {"1/32"s, ""s, 0.125},                            // 3
        {"1/16 triplet"s, "1/24"s, 0.16666667163372},     // 4
        {"1/32 dot"s, "3/64"s, 0.1875},                   // 5
        {"1/16 quintuplet"s, "1/20"s, 0.200000002980232}, // 6
        {"1/16"s, ""s, 0.25},                             // 7
        {"5/16"s, ""s, 0.3125},                           // 8
        {"1/8 triplet"s, "1/12"s, 0.333333343267441},     // 9
        {"1/16 dot"s, "3/32"s, 0.375},                    // 10
        {"1/8 quintuplet"s, "1/10"s, 0.400000005960464},  // 11
        {"1/16 doubledot"s, "5/64"s, 0.4375},             // 12
        {"1/8"s, ""s, 0.5},                               // 13
        {"9/16"s, ""s, 0.5625},                           // 14
        {"5/8"s, ""s, 0.625},                             // 15
        {"1/4 triplet"s, "1/6"s, 0.666666686534882},      // 16
        {"11/16"s, ""s, 0.6875},                          // 17
        {"1/8 dot"s, "3/16"s, 0.75},                      // 18
        {"1/4 quintuplet"s, "1/5"s, 0.800000011920929},   // 19
        {"13/16"s, ""s, 0.8125},                          // 20
        {"1/8 doubledot"s, "5/32"s, 0.875},               // 21
        {"15/16"s, ""s, 0.9375},                          // 22
        {"1/4"s, ""s, 1},                                 // 23
        {"17/16"s, ""s, 1.0625},                          // 24
        {"9/8"s, ""s, 1.125},                             // 25
        {"19/16"s, ""s, 1.1875},                          // 26
        {"5/4"s, ""s, 1.25},                              // 27
        {"21/16"s, ""s, 1.3125},                          // 28
        {"1/2 triplet"s, "1/3"s, 1.33333337306976},       // 29
        {"11/8"s, ""s, 1.375},                            // 30
        {"23/16"s, ""s, 1.4375},                          // 31
        {"1/4 dot"s, "3/8"s, 1.5},                        // 32
        {"25/16"s, ""s, 1.5625},                          // 33
        {"1/2 quintuplet"s, "2/5"s, 1.60000002384186},    // 34
        {"13/8"s, ""s, 1.625},                            // 35
        {"27/16"s, ""s, 1.6875},                          // 36
        {"1/4 doubledot"s, "5/16"s, 1.75},                // 37
        {"29/16"s, ""s, 1.8125},                          // 38
        {"15/8"s, ""s, 1.875},                            // 39
        {"31/16"s, ""s, 1.9375},                          // 40
        {"1/2"s, ""s, 2},                                 // 41
        {"1 triplet"s, "2/3"s, 2.66666674613953},         // 42
        {"1/2 dot"s, "3/4"s, 3},                          // 43
        {"1 quintuplet"s, "4/5"s, 3.20000004768372},      // 44
        {"1/2 doubledot"s, "5/8"s, 3.5},                  // 45
        {"1"s, ""s, 4},                                   // 46
        {"2 triplet"s, "4/3"s, 5.33333349227905},         // 47
        {"1 dot"s, "3/2"s, 6},                            // 48
        {"2 quintuplet"s, "8/5"s, 6.40000009536743},      // 49
        {"1 doubledot"s, "5/4"s, 7},                      // 50
        {"2"s, ""s, 8},                                   // 51
        {"4 triplet"s, "8/3"s, 10.6666669845581},         // 52
        {"2 dot"s, "3/1"s, 12},                           // 53
        {"4 quintuplet"s, "16/5"s, 12.8000001907349},     // 54
        {"2 doubledot"s, "5/2"s, 14},                     // 55
        {"4"s, ""s, 16},                                  // 56
        {"4 dot"s, "6/1"s, 24},                           // 57
        {"4 doubledot"s, "5/1"s, 28},                     // 58
    }};

  public:
    using Diffusor = DSP::DiffusorDelayChain<5000, 5>;

    explicit KindOfADelay(float sampleRate)
        : m_delay{DSP::DigitalDelay<maxDelayTimeInMilliseconds>(sampleRate),
                  DSP::DigitalDelay<maxDelayTimeInMilliseconds>(sampleRate)}
        , m_filter{DSP::MultiModeFourPoleMixerModule(sampleRate), DSP::MultiModeFourPoleMixerModule(sampleRate)}
        , m_diffusor{Diffusor(sampleRate), Diffusor(sampleRate)}
    {
        m_diffusor[0].setElementSize(0, 172);
        m_diffusor[0].setElementSize(1, 229);
        m_diffusor[0].setElementSize(2, 447);
        m_diffusor[0].setElementSize(3, 611);
        m_diffusor[0].setElementSize(4, 1176);
        m_diffusor[1].setElementSize(0, 182);
        m_diffusor[1].setElementSize(1, 219);
        m_diffusor[1].setElementSize(2, 437);
        m_diffusor[1].setElementSize(3, 631);
        m_diffusor[1].setElementSize(4, 1098);
        setModulationDepth(0.03f);
        setModulationSpeed(0.3f);
    }

    void setRhythmLeft(size_t index)
    {
        m_beats[0] = static_cast<float>(m_beatsList[index].beats);
        m_delay[0].setTime(60.f / m_bpm * m_beats[0]);
    }

    void setRhythmRight(size_t index)
    {
        m_beats[1] = static_cast<float>(m_beatsList[index].beats);
        m_delay[1].setTime(60.f / m_bpm * m_beats[1]);
    }

    void setBpm(float bpm)
    {
        m_bpm = bpm;
        m_delay[0].setTime(60.f / m_bpm * m_beats[0]);
        m_delay[1].setTime(60.f / m_bpm * m_beats[1]);
    }

    void setMix(float mix)
    {
        m_mix = DSP::getMixFactor(mix);
    }

    void setTimeInMillisecondsLeft(size_t milliseconds)
    {
        m_delay[0].setTime(static_cast<float>(milliseconds) / 1000.f);
    }

    void setTimeInMillisecondsRight(size_t milliseconds)
    {
        m_delay[1].setTime(static_cast<float>(milliseconds) / 1000.f);
    }

    void setFeedback(float feedback)
    {
        m_feedback = feedback;
    }

    void setCrossFeedback(float feedback)
    {
        m_crossFeedback = feedback;
    }

    void setFilterCutoff(float cutoff)
    {
        for (auto& filter : m_filter)
        {
            filter.setCutoff(cutoff);
        }
    }

    void setDiffuse(float nix)
    {
        m_diffusor[0].setMix(nix);
        m_diffusor[1].setMix(nix);
    }

    void setModulationDepth(float depth)
    {
        m_delay[0].setModulationDepth(depth);
        m_delay[1].setModulationDepth(depth);
    }

    void setModulationSpeed(float speed)
    {
        m_delay[0].setModulationSpeed(speed);
        m_delay[1].setModulationSpeed(speed * 0.9f); // slightly slower
    }

    void processBlock(const float* inLeft, const float* inRight, float* outLeft, float* outRight, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            feed(inLeft[i], inRight[i], outLeft[i], outRight[i]);
        }
    }

  private:
    void processFixedBlock(const float* inLeft, const float* inRight, float* outLeft, float* outRight)
    {
        for (size_t i = 0; i < InternalBlockSize; ++i)
        {
            m_tmpFeedback[0][i] = m_tmpFeed[0][i] * m_feedback + m_tmpFeed[1][i] * m_crossFeedback + inLeft[i];
            m_tmpFeedback[1][i] = m_tmpFeed[1][i] * m_feedback + m_tmpFeed[0][i] * m_crossFeedback + inRight[i];
        }
        m_filter[0].processBlock(m_tmpFeedback[0].data(), InternalBlockSize);
        m_filter[1].processBlock(m_tmpFeedback[1].data(), InternalBlockSize);
        m_diffusor[0].processBlock(m_tmpFeedback[0].data(), m_tmpDiffuse[0].data(), InternalBlockSize);
        m_diffusor[1].processBlock(m_tmpFeedback[1].data(), m_tmpDiffuse[1].data(), InternalBlockSize);

        m_delay[0].processBlock(m_tmpDiffuse[0].data(), m_tmpFeed[0].data(), InternalBlockSize);
        m_delay[1].processBlock(m_tmpDiffuse[1].data(), m_tmpFeed[1].data(), InternalBlockSize);
        for (size_t i = 0; i < InternalBlockSize; ++i)
        {
            outLeft[i] = inLeft[i] * m_mix.left + m_tmpFeed[0][i] * m_mix.right;
            outRight[i] = inRight[i] * m_mix.left + m_tmpFeed[1][i] * m_mix.right;
        }
    }

    void feed(float left, float right, float& outLeft, float& outRight)
    {
        m_tmpIn[0][fillPos] = left;
        m_tmpIn[1][fillPos] = right;
        outLeft = m_tmpOut[0][fillPos];
        outRight = m_tmpOut[1][fillPos];
        fillPos++;
        if (fillPos >= InternalBlockSize)
        {
            processFixedBlock(m_tmpIn[0].data(), m_tmpIn[1].data(), m_tmpOut[0].data(), m_tmpOut[1].data());
            fillPos = 0;
        }
    }

    std::array<std::array<float, InternalBlockSize>, 2> m_tmpIn{};
    std::array<std::array<float, InternalBlockSize>, 2> m_tmpOut{};
    size_t fillPos{0};

    std::array<std::array<float, InternalBlockSize>, 2> m_tmpFeed{};
    std::array<std::array<float, InternalBlockSize>, 2> m_tmpFeedback{};
    std::array<std::array<float, InternalBlockSize>, 2> m_tmpDiffuse{};
    DSP::PanValues<float> m_mix{0.7, 0.7f};
    float m_feedback{0.3f};
    float m_crossFeedback{0.1f};
    std::array<DSP::DigitalDelay<maxDelayTimeInMilliseconds>, 2> m_delay;
    std::array<DSP::MultiModeFourPoleMixerModule, 2> m_filter;
    float m_bpm{120.0f};
    std::array<float, 2> m_beats{0.25f, 0.25f};
    std::array<Diffusor, 2> m_diffusor;
};