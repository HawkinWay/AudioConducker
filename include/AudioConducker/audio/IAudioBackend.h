#pragma once

#include "AudioStream.h"
#include <vector>

namespace AudioConducker{

class IAudioBackend{
public:
    virtual ~IAudioBackend() = default;

    virtual std::vector<AudioStream> getStreams() = 0;

    virtual void setVolume(StreamId id, float volume) = 0;
};

} // namespace AudioConducker