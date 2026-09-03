#pragma once

#include "AudioConducker/audio/IAudioBackend.h"
#include "AudioConducker/core/Logger.h"
#include <vector>
#include <unordered_map>
#include <optional>

namespace AudioConducker{

class DuckingEngine{
public:
    DuckingEngine(AudioBackend& backend, float duckLevel = 0.2f);

    void process(std::optional<StreamId> focusStream);
    
private:
	void restore();
    
	void duck(StreamId focusStream, const std::vector<AudioStream>& streams);

    AudioBackend& backend_;
    float duckLevel_;
    std::unordered_map<StreamId, float> originalVolumes_;
    bool isActive_{false};
};

} // namespace AudioConducker
