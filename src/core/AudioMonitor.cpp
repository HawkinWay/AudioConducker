#include "AudioConducker/core/AudioMonitor.h"

namespace AudioConducker{

AudioMonitor::AudioMonitor(AudioBackend& backend) : backend_(backend){}

void AudioMonitor::update(){
    streams_ = backend_.getStreams();
}

std::vector<AudioStream> AudioMonitor::getActiveStreams() const{
    return streams_;
}

std::optional<StreamId> AudioMonitor::findStreamByApplication(const std::string& application) const{
    for(const auto& stream : streams_){
        if(stream.application == application){
            return stream.id;
        }
    }
    return std::nullopt;
}

} // namespace AudioConducker