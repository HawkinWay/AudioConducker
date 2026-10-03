#pragma once

#include "AudioConducker/audio/IAudioBackend.h"
#include "AudioConducker/core/Logger.h"
#include "AudioConducker/core/VolumeSmoother.h"
#include <vector>
#include <unordered_map>
#include <optional>

using Clock = std::chrono::steady_clock;

namespace AudioConducker{

class DuckingEngine{
public:
    DuckingEngine(
        IAudioBackend& backend, 
        float duckAmount = 0.5f, 
        float attackTime = 0.08f, 
        float releaseTime = 0.5f, 
        float holdTime = 0.4f
    );

    void process(std::optional<StreamId> focusStream, float deltaTime);
    
    void shutDown();

private:
    void updateDucking(std::optional<StreamId> focusStream, const std::vector<AudioStream>& streams, float deltaTime);
    
    void updateRestore(float deltaTime);

    void syncWithStreams(std::optional<StreamId> focusStream, const std::vector<AudioStream>& streams);

    IAudioBackend& backend_;
    
    float duckAmount_;
    float attackTime_;
    float releaseTime_;
    //std::chrono::milliseconds holdTime_;
    float holdTime_;

    // std::optional<Clock::time_point> lastTime_;
    float sinceLastActive_ = std::numeric_limits<float>::max();

    struct StreamState{
        float originalVolume{1.f};
        VolumeSmoother smoother{0.1f, 0.3f};
    };
    std::unordered_map<StreamId, StreamState> states_;

};

} // namespace AudioConducker
