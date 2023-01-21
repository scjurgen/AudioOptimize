#include "gtest/gtest.h"

#include "BeatsList.h"
#include <random>


#include <algorithm>
#include <array>
#include <cmath>

TEST(DspGenerator, beats)
{
    BusinessLogic::BeatsList beatsList;
    beatsList.generate();
    beatsList.print();
}
