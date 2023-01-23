#include "BufferInterpolation.h"
#include "DspPerformance.h"

#include "gtest/gtest.h"

#include <array>
#include <iostream>

namespace DspPerformanceTest
{

class Runner
{
  public:
    template <typename F>
    size_t process(float seconds, F interpolator)
    {
        constexpr size_t blockSize = 960;
        std::array<float, blockSize + 6> source{};

        std::minstd_rand generator(42);
        std::uniform_real_distribution<float> distribution(-1.f, 1.f);
        std::generate(source.begin(), source.end(), [&generator, &distribution]() { return distribution(generator); });

        auto start = std::chrono::steady_clock::now();
        const size_t numBlocks = seconds * 48000 / blockSize;
        for (size_t n = 0; n < numBlocks; ++n)
        {
            for (size_t j = 0; j < blockSize; ++j)
            {
                source[j] = interpolator(source.data() + j, source[0]);
            }
        }
        return numBlocks * blockSize;
    }
};

TEST(BufferInterpolationPerformanceTest, performance)
{
    Runner sut;
    auto start = std::chrono::steady_clock::now();
    const float seconds = 100;
    sut.process(seconds, DSP::bspline_43z);
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    auto msecs = static_cast<double>(duration.count()) / 1000.0;
    std::cout << "BufferInterpolationPerformanceTest.performance: " << msecs << " ms ";
    auto secondsNeeded = static_cast<double>(duration.count()) / 1'000'000.;
    std::cout << "\tload of " << secondsNeeded * 100.f / seconds << " % per thread" << std::endl;
#ifdef NDEBUG
    EXPECT_LT(msecs, 20);
#else
    EXPECT_LT(msecs, 100);
#endif
}


TEST(BufferInterpolationPerformanceTest, compareOptimized)
{
    constexpr size_t iterationsPerProcess{10};
    class SUTBase
    {
      public:
        SUTBase() = default;
        ~SUTBase() = default;
        void process()
        {
            m_samplesProcessed += sut.process(10, DSP::bspline_43z);
        }
        [[nodiscard]] size_t samplesProcessed() const
        {
            return m_samplesProcessed;
        }

      private:
        Runner sut;
        size_t m_samplesProcessed{0};
    };

    class SUTOptimized
    {
      public:
        SUTOptimized() = default;
        ~SUTOptimized() = default;
        void process()
        {
            m_samplesProcessed += sut.process(10, DSP::bspline_43x);
        }

        [[nodiscard]] size_t samplesProcessed() const
        {
            return m_samplesProcessed;
        }

      private:
        Runner sut;
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
    sut.printResult(sutOptimized.samplesProcessed(), oneBurnInSeconds, 48000.f);

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
