#include <gtest/gtest.h>

#include <array>

#include "AudioConducker/audio/IAudioBackend.h"
#include "AudioConducker/core/ConfigManager.h"
#include "AudioConducker/core/DuckingEngine.h"

using namespace AudioConducker;

class MockBackend : public IAudioBackend{
public:
    std::vector<AudioStream> streams;
    std::map<StreamId, float> applied;
    int setVolumeCalls{0};

    std::vector<AudioStream> getStreams() override{
        return streams;
    }

    void setVolume(StreamId id, float volume) override{
        applied[id] = volume;
        ++setVolumeCalls;
        for(auto& s : streams) if(s.id == id) s.volume = volume;
    }
};

class DuckingEngineTestClass: public ::testing::Test{
protected:
    static constexpr float kDuckAmount  = 0.5f;
    static constexpr float kAttackTime  = 0.08f;
    static constexpr float kReleaseTime = 0.5f;
    static constexpr float kHoldTime    = 0.4f;

    static constexpr float kDt = 0.02f;
    static constexpr float kRestoreTolerance = 1e-4f;

    MockBackend backend;
    ConfigManager config;
    DuckingEngine engine{backend, kDuckAmount, kAttackTime, kReleaseTime, kHoldTime};

    AudioStream makeStream(StreamId id, float vol, bool isActive, bool controllable = true){
        AudioStream stream = {
            .id = id,
            .name = "node",
            .application = "name",
            .mediaClass = "Stream/Output/Audio",
            .mediaName = "media",
            .volume = vol,
            .channelCount = 2,
            .isActive = isActive,
            .controllable = controllable,
        };

        return stream; 
    }

    void run(int frames, std::optional<StreamId> focusStream){
        for(int i = 0; i < frames; i++){
            engine.process(focusStream, kDt);
        }
    }
};




TEST_F(DuckingEngineTestClass, DucksNonFocusStreamsOnly){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 1.f, false), makeStream(3, 1.f, false, /*controllable=*/false) };
    
    run(10, 1);

    EXPECT_NEAR(backend.applied[2], 1.f * (1.f - kDuckAmount), kRestoreTolerance);
    EXPECT_EQ(backend.applied.count(1), 0); 
    EXPECT_EQ(backend.applied.count(3), 0);
}

TEST_F(DuckingEngineTestClass, UncontrollableStreamsSkipped){
    backend.streams = { makeStream(1, 1.f, true),
                        makeStream(2, 1.f, false, /*controllable=*/false) };
    engine.process(1, 0.02f);
    EXPECT_EQ(backend.applied.count(2), 0);
}

TEST_F(DuckingEngineTestClass, DuckingIsGradualNotInstant){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 1.f, false) };

    engine.process(1, kDt);
    EXPECT_GT(backend.applied[2], 1.f - kDuckAmount);
    EXPECT_LT(backend.applied[2], 1.f);

    run(10, 1);
    EXPECT_FLOAT_EQ(backend.applied[2], 1.f - kDuckAmount);
}

TEST_F(DuckingEngineTestClass, HoldPreventsImmediateRestore){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 0.5f, false) };
    
    run(10, 1);                                        // duck 收敛
    ASSERT_FLOAT_EQ(backend.applied[2], 0.5f * (1.f - kDuckAmount)); 

    backend.streams[1].isActive = false;

    run(10, std::nullopt);
    EXPECT_FLOAT_EQ(backend.applied[2], 0.25f);

    run(100, std::nullopt);
    // EXPECT_FLOAT_EQ(backend.applied[2], 0.5f); 
    // Before remove stead_clock in DuckingEngine.This won't pass, 
    // backend.applied[2] is always 0.25f, means restore never happened
    // Because hold use steady_clock, but test will run it in milliseconds
    EXPECT_NEAR(backend.applied[2], 0.5f, kRestoreTolerance);
}

TEST_F(DuckingEngineTestClass, DuckTargetUsesConfiguredFractionOfInitialVolume){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 0.8f, false) };

    run(10, 1);

    ASSERT_TRUE(backend.applied.contains(2));
    EXPECT_NEAR(backend.applied.at(2), 0.8f * (1.f - kDuckAmount), kRestoreTolerance);
    EXPECT_NEAR(backend.streams[1].volume, 0.4f, kRestoreTolerance);
}

TEST(DuckingEngineRestorationTest, RestoresSeveralInitialVolumesAfterFocusStops){
    constexpr std::array<float, 3> initialVolumes{0.2f, 0.5f, 1.f};
    constexpr float duckAmount = 0.5f;
    constexpr float attackTime = 0.08f;
    constexpr float releaseTime = 0.5f;
    constexpr float holdTime = 0.4f;
    constexpr float dt = 0.02f;
    constexpr float tolerance = 1e-4f;

    for(const float initialVolume : initialVolumes){
        MockBackend localBackend;
        localBackend.streams = {
            AudioStream{.id = 1, .name = "focus", .application = "focus",
                        .mediaClass = "Stream/Output/Audio", .mediaName = "focus",
                        .volume = 1.f, .channelCount = 2, .isActive = true, .controllable = true},
            AudioStream{.id = 2, .name = "background", .application = "background",
                        .mediaClass = "Stream/Output/Audio", .mediaName = "background",
                        .volume = initialVolume, .channelCount = 2, .isActive = true, .controllable = true},
        };
        DuckingEngine localEngine{localBackend, duckAmount, attackTime, releaseTime, holdTime};

        for(int frame = 0; frame < 10; ++frame) localEngine.process(1, dt);
        const float expectedTarget = initialVolume * (1.f - duckAmount);
        ASSERT_NEAR(localBackend.streams[1].volume, expectedTarget, tolerance)
            << "initial volume=" << initialVolume;

        localBackend.streams[0].isActive = false;
        for(int frame = 0; frame < 80; ++frame) localEngine.process(1, dt);
        EXPECT_NEAR(localBackend.streams[1].volume, initialVolume, tolerance)
            << "initial volume=" << initialVolume;
    }
}

TEST_F(DuckingEngineTestClass, HoldCompletesPartialDuckTransition){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 1.f, false) };

    engine.process(1, kDt);
    backend.streams[1].isActive = false;
    
    run(10, std::nullopt);
    EXPECT_FLOAT_EQ(backend.applied[2], 1.f - kDuckAmount);
}

TEST_F(DuckingEngineTestClass, InactiveFocusRestoresAfterHoldAndRelease){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 0.5f, false) };
    run(10, 1);
    ASSERT_NEAR(backend.streams[1].volume, 0.25f, kRestoreTolerance);

    backend.streams[0].isActive = false;
    run(80, 1); // Focus ID is still present, but its stream is inactive.

    EXPECT_NEAR(backend.streams[1].volume, 0.5f, kRestoreTolerance);
}

TEST_F(DuckingEngineTestClass, MissingFocusStreamRestoresAfterHoldAndRelease){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 0.5f, false) };
    run(10, 1);
    ASSERT_NEAR(backend.streams[1].volume, 0.25f, kRestoreTolerance);

    backend.streams.erase(backend.streams.begin()); // Focus ID no longer resolves.
    run(80, 1);

    EXPECT_NEAR(backend.streams[0].volume, 0.5f, kRestoreTolerance);
}

TEST_F(DuckingEngineTestClass, NoVolumeCallsAfterRestoreSettles){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 1.f, false) };

    run(10, 1);
    run(150, std::nullopt);                // hold(400ms) + restore(500ms) = 0.9s → 45 frames，150 is enough
    ASSERT_FLOAT_EQ(backend.applied[2], 1.f);   // ensure restored

    backend.setVolumeCalls = 0;
    run(20, std::nullopt);
    EXPECT_EQ(backend.setVolumeCalls, 0) << "engine should stop calling setVolume once settled";
}

TEST_F(DuckingEngineTestClass, StreamVanishingMidDuckIsHandled){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 1.f, false) };

    run(1, 1);
    ASSERT_GT(backend.applied[2], 0.5f);
    const float lastAppliedVolume = backend.applied.at(2);
    const int callsBefore = backend.setVolumeCalls;

    backend.streams.pop_back();            // stream 2 disappears

    EXPECT_NO_THROW(run(10, 1));           // no crash, no exception
    EXPECT_EQ(backend.setVolumeCalls - callsBefore, 0) << "vanished stream must receive no further setVolume calls";
    EXPECT_FLOAT_EQ(backend.applied.at(2), lastAppliedVolume);
}

TEST_F(DuckingEngineTestClass, ShutDownRestoresOriginalVolumeImmediately){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 0.5f, false) };
    run(10, 1);
    ASSERT_NEAR(backend.streams[1].volume, 0.25f, kRestoreTolerance);

    engine.shutDown();

    EXPECT_NEAR(backend.streams[1].volume, 0.5f, kRestoreTolerance);
}
