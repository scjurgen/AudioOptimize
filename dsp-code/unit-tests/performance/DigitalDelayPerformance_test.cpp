
#include "gtest/gtest.h"

#include "DigitalDelay.h"
#include "DspPerformance.h"

#include <chrono>

namespace DspPerformanceTest
{

TEST(DigitalDelayPerformanceTest, performance)
{
    constexpr size_t seconds = 10;
    // settle parameters for some seconds
    constexpr size_t numSamples = 48000 * seconds;
    constexpr size_t blockSize = 128;
    constexpr size_t numBlocks = numSamples / blockSize;

    DSP::DigitalDelay<1000> sut(48000.0f);
    std::array<float, blockSize> source{};
    source[0] = 1.f;

    auto start = std::chrono::steady_clock::now();
    for (size_t j = 0; j < numBlocks; ++j)
    {
        source[0] = 1.f;
        sut.processBlock(source.data(), source.data(), 128);
    }
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    auto msecs = static_cast<double>(duration.count()) / 1000.0;
    std::cout << "DigitalDelayPerformanceTest.performance: " << msecs << " ms";
    auto secondsNeeded = static_cast<double>(duration.count()) / 1'000'000.;
    std::cout << "\tload of " << secondsNeeded * 100.f / seconds << " % per thread" << std::endl;
#if NDEBUG
#ifdef __VERSION__
    // docker debian bookworm
    if (std::string(__VERSION__).find(std::string("11.2")) != std::string::npos)
    {
        EXPECT_NEAR(66, msecs, 20); // add tolerance for virtual machine
    }
    // local build mac
    else if (std::string(__VERSION__).find("Apple LLVM 14.") != std::string::npos)
    {
        EXPECT_NEAR(25, msecs, 5);
    }
    else
    {
        std::cerr << "THIS VERSION IS NOT TESTED AND NEEDS CALIBRATION: " << __VERSION__ << std::endl;
    }
#endif
#ifdef _MSC_VER
    std::cerr << "THIS MSC VERSION IS NOT TESTED AND NEEDS CALIBRATION: " << _MSC_VER << std::endl;
#endif
#else
    std::cerr << __FILE_NAME__ << ": Warning! Debug version will not be tested!" << std::endl;
#endif
}

TEST(DigitalDelayPerformanceTest, compareOlder)
{
#if !NDEBUG
    std::cerr << __FILE_NAME__ << ": Warning! Debug version should not be compared!" << std::endl;
#endif

    constexpr size_t iterationsPerProcess{10};
    constexpr float sampleRate{48000.f};
    class SUTBase
    {
      public:
        SUTBase()
            : sut(sampleRate)
        {
        }
        void process()
        {
            for (size_t i = 0; i < iterationsPerProcess; ++i)
            {
                m_data[0] = 1.f;
                sut.processBlock(m_data.data(), m_data.data(), m_data.size());
                EXPECT_NE(m_data[0], 20);
            }
            m_samplesProcessed += iterationsPerProcess * m_data.size();
        }

        [[nodiscard]] size_t samplesProcessed() const
        {
            return m_samplesProcessed;
        }

      private:
        DSP::DigitalDelay<1000> sut;
        std::array<float, 1024> m_data{};
        size_t m_samplesProcessed{0};
    };

    class SUTOptimized
    {
      public:
        SUTOptimized()
            : sut(sampleRate)
        {
        }

        void process()
        {
            for (size_t i = 0; i < iterationsPerProcess; ++i)
            {
                m_data[0] = 1.f;
                sut.processBlock(m_data.data(), m_data.data(), m_data.size());
                EXPECT_NE(m_data[0], 20);
            }
            m_samplesProcessed += iterationsPerProcess * m_data.size();
        }

        [[nodiscard]] size_t samplesProcessed() const
        {
            return m_samplesProcessed;
        }

      private:
        DSP::DigitalDelayOptimized<1000> sut;
        std::array<float, 1024> m_data{};
        size_t m_samplesProcessed{0};
    };

    const auto seconds = .5f;
    SUTBase sutBase;
    SUTOptimized sutOptimized;
    const auto baseRunner = [&sutBase]() { sutBase.process(); };
    const auto optimizedRunner = [&sutOptimized]() { sutOptimized.process(); };

    TestCompare testCompare;
    const auto iterationsForOneRound = testCompare.getIterationsForACertainPeriod(baseRunner, seconds);

    testCompare.runSingleTest(baseRunner, optimizedRunner, iterationsForOneRound);
    testCompare.printResult(sutOptimized.samplesProcessed(), seconds, sampleRate);
}

}
