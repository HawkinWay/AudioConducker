#include <gtest/gtest.h>

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

    EXPECT_FLOAT_EQ(backend.applied[2], 1.f - config.getDuckAmount());
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
    EXPECT_FLOAT_EQ(backend.applied[2], 0.5f);  // Before remove stead_clock in DuckingEngine.This won't pass, 
    // backend.applied[2] is always 0.25f, means restore never happened
    // Because hold use steady_clock, but test will run it in milliseconds
}

TEST_F(DuckingEngineTestClass, HoldCompletesPartialDuckTransition){
    backend.streams = { makeStream(1, 1.f, true), makeStream(2, 1.f, false) };

    engine.process(1, kDt);
    backend.streams[1].isActive = false;
    
    run(10, std::nullopt);
    EXPECT_FLOAT_EQ(backend.applied[2], 1.f - kDuckAmount);
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
    const int callsBefore = backend.setVolumeCalls;

    backend.streams.pop_back();            // stream 2 disappears

    EXPECT_NO_THROW(run(10, 1));           // no crash, no exception
    EXPECT_EQ(backend.setVolumeCalls - callsBefore, 0) << "vanished stream must receive no further setVolume calls";
    EXPECT_FLOAT_EQ(backend.applied[2], /* last value before vanishing */
                    backend.applied.at(2));
}
