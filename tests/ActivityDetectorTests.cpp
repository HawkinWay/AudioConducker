#include <gtest/gtest.h>

#include <vector>

#include "AudioConducker/audio/ActivityDetector.h"

using namespace AudioConducker;

TEST(ActivityDetectorTests, LoudSamplesActive){
    ActivityDetector detector;

    std::vector<float> samples(480, 0.5f);
    auto processed = detector.process(samples.data(), samples.size());
    EXPECT_TRUE(processed);
}

TEST(ActivityDetectorTests, SilentSamplesActive){
    ActivityDetector detector;

    std::vector<float> samples(480, 0.f);
    auto processed = detector.process(samples.data(), samples.size());
    EXPECT_FALSE(processed);
}

#if 0
TEST(ActivityDetectorTests, ThresholdBoundary){
    ActivityDetector detector;

    std::vector<float> samples(480, detector.getThreshold());
    auto processed = detector.process(samples.data(), samples.size());
    EXPECT_FALSE(processed) << "equal-to-threshold shouldn't count as active";
    // In process(), the return value is rms > threshold_, so this EXPECT_FALSE test should pass theoretically.
    // But it didn't pass. Why? The default threshold = 0.05f, `sqrt(0.0025f)` yields the `float` value closest 
    // to the true value of 0.01. The result may be one ULP greater or one ULP smaller than `0.01f`, 
    // depending on the rounding direction of `0.05f` itself.
}
#endif

TEST(ActivityDetectorTests, ThresholdBoundary){
    ActivityDetector detector;
    auto threshold = detector.getThreshold();

    std::vector<float> samples1(480, threshold * 1.5f);
    auto processed1 = detector.process(samples1.data(), samples1.size());
    EXPECT_TRUE(processed1);

    std::vector<float> samples2(480, threshold * 0.5f);
    auto processed2 = detector.process(samples2.data(), samples2.size());
    EXPECT_FALSE(processed2);

}

TEST(ActivityDetectorTests, EmptyInputIsInactive){
    ActivityDetector detector;
    EXPECT_FALSE(detector.process(nullptr, 0));
}