
#pragma once

#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

namespace DspPerformanceTest
{
/*
 * we need a process that just burns data. In this way we can run two threads in parallel that will (hopefully)
 * be sliced similarly when running the performance test
 */
class BurnData
{
  public:
    BurnData()
    {
        // ensure 0 ~< m_someData ~< 1
        m_someData = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.9 + 0.05;
    }

    void burn(size_t multiplications, size_t blocksize)
    {
        for (size_t frame = 0; frame < blocksize; ++frame)
        {
            for (size_t m = 0; m < multiplications; ++m)
            {
                m_someData = 3.999f * m_someData * (1.f - m_someData); // logistic function, creates chaos
                /* for comparison
                 /// clang 13 unrolls >=4 to from the inner loop repeating this fragment 4 times (so the intermediate
                 value does not get stored): mulss   xmm3, xmm0 mulss   xmm3, xmm2 movaps  xmm2, xmm1 subss   xmm2, xmm3
                 /// 2 multiplications 1 substraction
                 /// msvc unrolls too, but more shifting around and always storing directly in target
                    movaps  xmm0, xmm4
                    subss   xmm0, xmm1
                    movaps  xmm2, xmm1
                    mulss   xmm2, xmm3
                    mulss   xmm2, xmm0
                    movss   DWORD PTR [rcx], xmm2
                 */
            }
        }
    }
    [[nodiscard]] float getValue() const
    {
        return m_someData;
    }

  private:
    float m_someData;
};


// compare two runners and compute the difference in performance

class TestCompare
{
  public:
    std::atomic<bool> terminateBase{false};
    std::atomic<bool> terminateCompare{false};

    uint64_t base(std::function<void()> baseRunner, uint64_t items)
    {
        terminateBase.store(false);
        for (uint64_t i = 0; i < items; ++i)
        {
            baseRunner();
            if (terminateBase.load())
            {
                return i + 1;
            }
        }
        terminateCompare.store(true);
        return items;
    }

    uint64_t compare(std::function<void()> testRunner, uint64_t items)
    {
        terminateCompare.store(false);
        for (uint64_t i = 0; i < items; ++i)
        {
            testRunner();
            if (terminateCompare.load())
            {
                return i + 1;
            }
        }
        terminateBase.store(true);
        return items;
    }

    void measureTimeBasedIterations(const double seconds)
    {
        terminateBase.store(false);
        auto start = std::chrono::steady_clock::now();
        while (true)
        {
            auto end = std::chrono::steady_clock::now();
            auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            auto s = usecs / 1'000'000.0L;
            if (s >= seconds)
            {
                terminateBase.store(true);
                break;
            }
        }
    }

    uint64_t getIterationsForACertainPeriod(std::function<void()> runner, double seconds)
    {
        auto baseThread = std::async(&TestCompare::base, this, runner, std::numeric_limits<uint64_t>::max());
        auto compareThread = std::async(&TestCompare::measureTimeBasedIterations, this, seconds);
        compareThread.get();
        return baseThread.get();
    }

    void runSingleTest(std::function<void()> baseRunner, std::function<void()> runner, uint64_t itemsForOneRound,
                       uint64_t& iterationsBase, uint64_t& iterationsCompare)
    {
        auto baseThread = std::async(&TestCompare::base, this, baseRunner, itemsForOneRound);
        auto compareThread = std::async(&TestCompare::compare, this, runner, itemsForOneRound);
        iterationsBase = baseThread.get();
        iterationsCompare = compareThread.get();
    }

    void runSingleTest(std::function<void()> baseRunner, std::function<void()> optimizedRunner,
                       uint64_t itemsForOneRound)
    {
        auto baseThread = std::async(&TestCompare::base, this, baseRunner, itemsForOneRound);
        auto compareThread = std::async(&TestCompare::compare, this, optimizedRunner, itemsForOneRound);
        m_iterationsBase = baseThread.get();
        m_iterationsCompare = compareThread.get();
    }

    void printResult(uint64_t samplesProcessed, float seconds, float sampleRate)
    {
        auto deltaPercent = m_iterationsCompare * 100 / m_iterationsBase;
        std::cout << "Baseline:   " << m_iterationsBase << " iterations (in " << seconds << " secs.)\n";
        std::cout << "Optimized:  " << m_iterationsCompare << " iterations\n";
        std::cout << "Difference: " << deltaPercent << "%";
        if (deltaPercent < 97)
        {
            std::cout << " (\033[01;31mdoing worse\033[0m)\n";
        }
        else if (deltaPercent > 103)
        {
            std::cout << " (\033[01;32mdoing better\033[0m)\n";
        }
        else
        {
            std::cout << " (\033[01;36mnot really a big difference\033[0m)\n";
        }
        std::cout << "Local speed factor: " << samplesProcessed / sampleRate / seconds;
        std::cout << " blocks/second (1 block equals " << sampleRate << " samples)\n" << std::endl;
#ifndef NDEBUG
        std::cout << " (\033[01;31mWARNING! You are testing the DEBUG version!\033[0m)\n";
#endif
    }

    uint64_t m_iterationsBase;
    uint64_t m_iterationsCompare;
};
}
