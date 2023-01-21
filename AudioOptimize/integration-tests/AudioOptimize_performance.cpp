
#include "unit-tests/DspPerformance.h"

#include "gtest/gtest.h"


namespace OT_test
{

TEST(AudioOptimizeIntegrationTests, basicsTestsVariations)
{
    constexpr size_t frameSize = 1024;

    Dsp::Performance::TestCompare tcmp;
    Dsp::Performance::BurnData burnData;

    // reduce burner throughput, gain processing is extremly quick!
    auto burner = [&burnData, &frameSize]() { burnData.burn(1, frameSize / 16); }; // capture for MSVC
    auto iterations = tcmp.getIterationsForACertainPeriod(burner, 0.1);
    auto runner = [&sut, &processingInfo]()
    {
        std::fill_n(processingInfo.audioInputs()[0].buffer()->data(), processingInfo.audioInputs()[0].buffer()->size(),
                    0.5f);
        sut.process(processingInfo);
    };
    uint64_t iterationsBase, iterationsCompare;
    tcmp.runSingleTest(burner, runner, iterations, iterationsBase, iterationsCompare);

    // check if values have been actually processed
    auto v = processingInfo.audioOutputs()[0].buffer()->data()[0];
    EXPECT_EQ(v, 0.25f);

    int ratioPercent = static_cast<int>(100 * iterationsCompare / iterationsBase);
    std::cout << "Iterations to do:" << iterations << " base: " << iterationsBase
              << " -> Compare: " << iterationsCompare << "   r: " << ratioPercent << " %" << std::endl;

#if NDEBUG
#ifdef __VERSION__
    // docker debian bookworm
    if (std::string(__VERSION__).find(std::string("11.2")) != std::string::npos)
    {
        EXPECT_NEAR(66, ratioPercent, 20);
    }
    // local build mac

    else if (std::string(__VERSION__).find("Apple LLVM 13.") != std::string::npos)
    {
        EXPECT_NEAR(90, ratioPercent, 20);
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
    std::cerr << "Debug version will not be tested" << std::endl;
#endif
}
}
