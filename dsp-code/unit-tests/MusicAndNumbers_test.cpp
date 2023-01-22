#include "MusicAndNumbers.h"

#include "gtest/gtest.h"

#include <array>
#include <iostream>

namespace MusicTest
{

TEST(MusicAndNumbersTest, midiNoteToFrequency)
{
    constexpr auto epsilon = 1.0E-3f;

    EXPECT_NEAR(Music::midiNoteToFrequency(33.f), 55.f, epsilon);
    EXPECT_NEAR(Music::midiNoteToFrequency(45.f), 110.f, epsilon);
    EXPECT_NEAR(Music::midiNoteToFrequency(57.f), 220.f, epsilon);
    EXPECT_NEAR(Music::midiNoteToFrequency(69.f), 440.f, epsilon);

    EXPECT_NEAR(Music::midiNoteToFrequency(48.f), 261.62546f / 2, epsilon);
    EXPECT_NEAR(Music::midiNoteToFrequency(60.f), 261.62546f, epsilon);
    EXPECT_NEAR(Music::midiNoteToFrequency(72.f), 2 * 261.62546f, epsilon);

    EXPECT_NEAR(Music::midiNoteToFrequency(69.5f), 452.89299f, epsilon);
}
}
