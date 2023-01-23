
#include "gtest/gtest.h"

#include "DspPerformance.h"
#include "OnePoleFilter.h"

#include <chrono>

namespace DspPerformanceTest
{

TEST(OnePoleFilterPerformanceTest, performance)
{
    constexpr size_t seconds = 120;
    // settle parameters for some seconds
    constexpr size_t numSamples = 48000 * seconds;
    constexpr size_t blockSize = 128;
    constexpr size_t numBlocks = numSamples / blockSize;

    DSP::OnePoleFilter sut(48000.0f);
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
    std::cout << "OnePoleFilterPerformanceTest.performance: " << msecs << " ms";
    auto secondsNeeded = static_cast<double>(duration.count()) / 1'000'000.;
    std::cout << "\tload of " << secondsNeeded * 100.f / seconds << " % per thread" << std::endl;
#ifdef NDEBUG
    EXPECT_LT(msecs, 25);
#else
    EXPECT_LT(msecs, 35);
#endif
}

TEST(OnePoleFilterPerformanceTest, compareOlder)
{
    constexpr size_t iterationsPerProcess{10};
    constexpr float sampleRate{48000.f};
    class SUTBase
    {
      public:
        SUTBase()
            : sut(sampleRate)
        {
            sut.setCutoff(12000.f);
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

        [[nodiscard]] size_t samplesProcessed() const
        {
            return m_samplesProcessed;
        }

      private:
        DSP::OnePoleFilter sut;
        std::array<float, 1024> m_data{};
        size_t m_samplesProcessed{0};
    };

    class SUTOptimized
    {
      public:
        SUTOptimized()
            : sut(sampleRate)
        {
            sut.setCutoff(12000.f);
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

        [[nodiscard]] size_t samplesProcessed() const
        {
            return m_samplesProcessed;
        }

      private:
        DSP::OnePoleFilterOptimized sut;
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

    sut.runSingleTest(baseRunner, optimizeRunner, iterationsToDo);
    sut.printResult(sutOptimized.samplesProcessed(), oneBurnInSeconds, sampleRate);
}

}
