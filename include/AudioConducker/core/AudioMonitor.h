#pragma once

#include "AudioConducker/core/Logger.h"
#include "AudioConducker/audio/IAudioBackend.h"
#include <string>
#include <optional>

namespace AudioConducker{

class AudioMonitor{
public:
    explicit AudioMonitor(AudioBackend& backend);

    void update();

    std::vector<AudioStream> getActiveStreams() const;

    std::optional<StreamId> findStreamByApplication(const std::string& application) const;

private:
    AudioBackend& backend_;
    std::vector<AudioStream> streams_;
};

} // namespace AudioConducker