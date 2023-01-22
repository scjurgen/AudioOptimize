
#include "DspPerformance.h"

#include "FourStageFilter.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace DspPerformanceTest
{

template <std::array<float, 5> f>
class FourStageMultiFilter2
{
  public:
    FourStageMultiFilter2(const float sampleRate)
        : m_sampleRate(sampleRate)
        , lp{sampleRate, sampleRate, sampleRate, sampleRate}
    {
        setCutoff(1000.f);
        m_stepsadvance = 0;
        m_pole = m_newpole;
    }

    void setSmoothingSteps(size_t steps)
    {
        m_stepsadvanceSetting = steps;
    }

    void setResonance(float value)
    {
        m_reso = value * 4.0f;
    }

    void setCutoff(const float cutoff)
    {
        m_newpole = std::exp(-2.0f * static_cast<float>(M_PI) * cutoff / m_sampleRate);
        m_stepsadvance = m_stepsadvanceSetting;
        m_advance = (m_newpole - m_pole) / static_cast<float>(m_stepsadvance);
        if (m_stepsadvanceSetting == 0)
        {
            m_pole = m_newpole;
        }
    }

    [[nodiscard]] float currentFactor() const
    {
        return m_pole;
    }

    void processBlock(float* source, size_t numSamples)
    {
        processBlock(source, source, numSamples);
    }

    void processBlock(const float* source, float* target, size_t numSamples)
    {
        size_t index = 0;
        size_t toIndex = numSamples;

        if (m_stepsadvance)
        {
            if (m_stepsadvance < numSamples)
            {
                toIndex = m_stepsadvance;
                m_stepsadvance = 0;
            }
            else
            {
                m_stepsadvance -= numSamples;
            }
            while (index < toIndex)
            {
                m_pole += m_advance;
                target[index] = singleStep(source[index]);
                ++index;
            }
            if (!m_stepsadvance)
            {
                m_pole = m_newpole;
            }
        }
        while (index < numSamples)
        {
            target[index] = singleStep(source[index]);
            ++index;
        }
    }

  private:
    float singleStep(const float in)
    {
        const float feed = in - m_stage4 * m_reso; // resonance could also be fed from the other 4 stages
        const float stage1 = lp[0].next(feed);
        const float stage2 = lp[1].next(stage1);
        const float stage3 = lp[2].next(stage2);
        m_stage4 = lp[3].next(stage3);
        float result = feed * f[0];
        result -= stage1 * f[1];
        result += stage2 * f[2];
        result -= stage3 * f[3];
        result += m_stage4 * f[4];
        return result;
    }

    float m_sampleRate;
    float m_advance{0};
    size_t m_stepsadvance{0};
    size_t m_stepsadvanceSetting{256u};
    float m_newpole{0.5};
    float m_reso{0.0};
    float m_pole{0.5};
    float m_stage4;
    std::array<DSP::OnePoleFilter, 4> lp;
};

TEST(FourPoleFilterPerformanceTest, performance)
{
    constexpr size_t seconds = 120;
    // settle parameters for some seconds
    constexpr size_t numSamples = 48000 * seconds;
    constexpr size_t blockSize = 128;
    constexpr size_t numBlocks = numSamples / blockSize;

    DSP::MultiModeFourPoleMixerModule sut(48000.f);
    std::array<float, blockSize> source{};
    source[0] = 1.f;
    sut.setCutoff(1000);

    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < numBlocks; ++j)
    {
        source[0] = 1.f;
        sut.processBlock(source.data(), 128);
    }
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    auto msecs = static_cast<double>(duration.count()) / 1000.0;
    std::cout << "FourPoleFilterPerformanceTest.performance: " << msecs << " ms";
    auto secondsNeeded = static_cast<double>(duration.count()) / 1'000'000.;
    std::cout << "\tload of " << secondsNeeded * 100.f / seconds << " % per thread" << std::endl;
#ifdef NDEBUG
    EXPECT_LT(msecs, 50);
#else
    EXPECT_LT(msecs, 280);
#endif
}


TEST(FourPoleFilterPerformanceTest, compareOlder)
{
    constexpr size_t iterationsPerProcess{10};
    class SUTBase
    {
      public:
        SUTBase()
            : sut(48000.f)
        {
            sut.setFactors(DSP::MultiModeDoubleNotch);
            sut.setCutoff(12000.f);
            sut.setResonance(0.2f);
        }
        void process()
        {
            for (size_t i = 0; i < iterationsPerProcess; ++i)
            {
                m_data[0] = 1.f;
                sut.processBlock(m_data.data(), m_data.size());
                EXPECT_NE(m_data[0], 0);
            }
            m_samplesProcessed += iterationsPerProcess * m_data.size();
        }

        size_t samplesProcessed()
        {
            return m_samplesProcessed;
        }

      private:
        DSP::MultiModeFourPoleMixerModule sut;
        std::array<float, 1024> m_data{};
        size_t m_samplesProcessed{0};
    };

    class SUTOptimized
    {
      public:
        SUTOptimized()
            : sut(48000.f)
        {
            sut.setCutoff(12000.f);
            sut.setResonance(0.2f);
        }

        void process()
        {
            for (size_t i = 0; i < iterationsPerProcess; ++i)
            {
                m_data[0] = 1.f;
                sut.processBlock(m_data.data(), m_data.size());
                EXPECT_NE(m_data[0], 0);
            }
            m_samplesProcessed += iterationsPerProcess * m_data.size();
        }

        size_t samplesProcessed()
        {
            return m_samplesProcessed;
        }

      private:
        FourStageMultiFilter2<DSP::MultiModeDoubleNotch> sut;
        std::array<float, 1024> m_data{};
        size_t m_samplesProcessed{0};
    };

    auto oneBurnInSeconds = .5f;
    SUTBase sutBase;
    SUTOptimized sutOptimized;
    auto baseRunner = [&sutBase]() { sutBase.process(); };
    auto optimizeRunner = [&sutOptimized]() { sutOptimized.process(); };

    TestCompare sut;
    auto iterationsToDo = sut.getIterationsForACertainPeriod(baseRunner, oneBurnInSeconds);
    uint64_t iterationsBase, iterationsOptimize;

    sut.runSingleTest(baseRunner, optimizeRunner, iterationsToDo, iterationsBase, iterationsOptimize);

    auto deltaPercent = iterationsOptimize * 100 / iterationsBase;
    std::cout << "Base: " << iterationsBase << " Optimized: " << iterationsOptimize;
    std::cout << " r: " << deltaPercent << "%";
    if (deltaPercent < 100)
    {
        std::cout << " (doing worse)" << std::endl;
    }
    else
    {
        std::cout << " (doing better)" << std::endl;
    }
    std::cout << "Local speed factor: " << sutOptimized.samplesProcessed() / 48000.f / oneBurnInSeconds << std::endl;
}
}
