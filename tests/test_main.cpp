#include <gtest/gtest.h>
#include "AudioConducker/audio/ActivityDetector.h"

TEST(ProjectTest, Basic)
{
    EXPECT_EQ(1,1);
}

TEST(ActivityDetectorTest, process){
    AudioConducker::ActivityDetector detector;

    float samples[] = {0.1f, 0.2f, 0.3f};
    
    EXPECT_TRUE(detector.process(samples, 3));
}
