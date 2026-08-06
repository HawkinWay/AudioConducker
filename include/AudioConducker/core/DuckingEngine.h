#pragma once

#include "AudioConducker/audio/IAudioBackend.h"

namespace AudioConducker{

class DuckingEngine{
public:
    explicit DuckingEngine(AudioBackend& backend);

    void process(StreamId focusStream);

private:
    AudioBackend& backend_;
    float duckVolume_ = 0.2f;
};

} // namespace AudioConducker