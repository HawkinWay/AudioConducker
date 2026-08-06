#pragma once

#include "AudioStream.h"
#include <vector>

namespace AudioConducker{

class AudioBackend{
public:
    virtual ~AudioBackend() = default;

    virtual std::vector<AudioStream> getStreams() = 0;

    virtual void setVolume(StreamId id, float volume) = 0;
};

} // namespace AudioConducker