#pragma once

#include "AudioConducker/core/Logger.h"
#include "AudioConducker/audio/IAudioBackend.h"
#include <vector>
#include <string>
#include <optional>
#include <unordered_set>

namespace AudioConducker{

class AudioMonitor{
public:
    explicit AudioMonitor(AudioBackend& backend);

    void update();
    void showNodes();
    void watchNodes();

    std::vector<AudioStream> getActiveStreams() const;

    std::optional<StreamId> findStreamByApplication(const std::string& application) const;

private:
    AudioBackend& backend_;
    std::vector<AudioStream> streams_;
    std::vector<AudioStream> previousStreams_;
};

} // namespace AudioConducker