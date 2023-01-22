
#include <iostream>

#include "gtest/gtest.h"

#include "DspPerformance.h"

namespace DspPerformanceTest
{

TEST(DSP_PerformanceTest_tests, basicsTestsVariations)
{
    class SUT
    {
      public:
        SUT()
        {
            m_someData = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }
        void burnData(size_t multiplications, size_t blocksize)
        {
            for (size_t frame = 0; frame < blocksize; ++frame)
            {
                for (size_t m = 0; m < multiplications; ++m)
                {
                    m_someData = 3.999f * m_someData * (1.f - m_someData); // logistic function, creates chaos
                }
            }
        }

      private:
        float m_someData;
    };

    std::vector<size_t> multiplicationsInRunner{3, 4, 5, 6, 8};
    constexpr size_t multiplicationsBase{5};
    for (auto multiplications : multiplicationsInRunner)
    {
        auto oneBurnInSeconds = 0.2;
        SUT sutBase;
        SUT sutRunner;
        // 5 multiplictions in base
        auto baseRunner = [&sutBase]() { sutBase.burnData(multiplicationsBase, 1024); };
        // e.g.: 4 multiplictions in compare --> 5:4 = 25% faster
        auto compareRunner = [&sutRunner, &multiplications]() { sutRunner.burnData(multiplications, 1024); };

        TestCompare sut;
        auto iterationsToDo = sut.getIterationsForACertainPeriod(baseRunner, oneBurnInSeconds);
        uint64_t iterationsBase, iterationsCompare;
        sut.runSingleTest(baseRunner, compareRunner, iterationsToDo, iterationsBase, iterationsCompare);

        auto deltaPercent = iterationsCompare * 100 / iterationsBase;
        // std::cout << "Base: " << iterationsBase << " Compare: " << iterationsCompare;
        // std::cout << " r: " << deltaPercent << "% vs:" << multiplicationsBase * 100 / multiplications
        //           << " err permitted: " << 10 * 5 / multiplications << std::endl;
        //  auto shouldBeGreaterThen = multiplicationsBase * 100 / multiplications;
        //  as our multiplications in the runner are faster the error will be greater (doesn't scale linear)
        EXPECT_NEAR(deltaPercent, multiplicationsBase * 100 / multiplications, 10 * 5 / multiplications);
    }
}
}
