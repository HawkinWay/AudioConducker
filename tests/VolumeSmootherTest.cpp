#include <gtest/gtest.h>

#include "AudioConducker/core/VolumeSmoother.h"

TEST(VolumeSmootherTests, targetDownward){
    AudioConducker::VolumeSmoother smoother(0.1f, 0.3f);

    smoother.setCurrent(1.f);
    smoother.setTarget(0.f);

    float volume = smoother.process(0.05f);

    EXPECT_FLOAT_EQ(volume, 0.5f);

    volume = smoother.process(0.05f);

    EXPECT_FLOAT_EQ(volume, 0.0f);

}

TEST(VolumeSmootherTests, doesNotOverShootTarget){
    AudioConducker::VolumeSmoother smoother(0.1f, 0.3f);

    smoother.setCurrent(1.f);
    smoother.setTarget(0.2f);

    float volume = smoother.process(0.05f);

    EXPECT_FLOAT_EQ(volume, 0.5f);

    volume = smoother.process(0.05f);

    EXPECT_FLOAT_EQ(volume, 0.2f);

    volume = smoother.process(0.05f);

    EXPECT_FLOAT_EQ(volume, 0.2f);

}

TEST(VolumeSmootherTests, usesReleaseTimeWhenIncreasing){
    AudioConducker::VolumeSmoother smoother(0.1f, 0.2f);

    smoother.setCurrent(0.2f);
    smoother.setTarget(1.f);

    float volume = smoother.process(0.1f);

    EXPECT_FLOAT_EQ(volume, 0.7f);
}

TEST(VolumeSmootherTest, CanReverseDirection)
{
    AudioConducker::VolumeSmoother smoother(0.1f, 0.3f);

    smoother.setCurrent(1.0f);
    smoother.setTarget(0.0f);

    // Start ducking
    float volume = smoother.process(0.05f);

    EXPECT_FLOAT_EQ(volume, 0.5f);

    // Focus stops → release
    smoother.setTarget(1.0f);

    volume = smoother.process(0.15f);

    EXPECT_FLOAT_EQ(volume, 1.0f);
}