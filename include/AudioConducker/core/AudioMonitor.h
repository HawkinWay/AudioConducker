#pragma once

#include "AudioConducker/audio/IAudioBackend.h"

namespace AudioConducker{

class AudioMonitor{
public:
    explicit AudioMonitor(AudioBackend& backend);

    void update();

    std::vector<AudioStream> getActiveStreams() const;
private:
    AudioBackend& backend_;
    std::vector<AudioStream> streams_;
};

} // namespace AudioConducker