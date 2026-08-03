#include "AudioConducker/core/AudioMonitor.h"

namespace AudioConducker{

AudioMonitor::AudioMonitor(AudioBackend& backend) : backend_(backend){}

void AudioMonitor::update(){
    streams_ = backend_.getStreams();
}

std::vector<AudioStream> AudioMonitor::getActiveStreams() const{
    return streams_;
}

} // namespace AudioConducker