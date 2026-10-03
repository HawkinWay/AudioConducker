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
    explicit AudioMonitor(IAudioBackend& backend);

    void update();
    void showNodes();
    void watchNodes();

    bool isTransient(const AudioStream& stream) const;

    std::vector<AudioStream> getActiveStreams() const;

    std::optional<StreamId> findStreamByApplication(const std::string& application) const;

private:
    IAudioBackend& backend_;
    std::vector<AudioStream> streams_;
    std::vector<AudioStream> previousStreams_;

    std::unordered_set<std::string> transientNames_ = {"AudioStream"};
};

} // namespace AudioConducker