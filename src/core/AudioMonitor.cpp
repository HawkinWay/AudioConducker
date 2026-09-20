#include "AudioConducker/core/AudioMonitor.h"

#include <iostream>
#include <iomanip>

namespace AudioConducker{

AudioMonitor::AudioMonitor(AudioBackend& backend) : backend_(backend){}

void AudioMonitor::update(){
    previousStreams_ = streams_;
    streams_ = backend_.getStreams();
}

void AudioMonitor::watchNodes(){
    for(const auto& current : streams_){
        const auto it = std::find_if(
            previousStreams_.begin(),
            previousStreams_.end(),
            [&](const AudioStream& previous){
                return current.id == previous.id;
            }
        );

        if(it == previousStreams_.end()){
            std::cout 
                << std::left
                << "[+] Node: " << std::setw(5)
                << current.id
                << current.application << "/"
                << current.mediaName << '\n';
        }
    }

    for(const auto& previous : previousStreams_){
        const auto it = std::find_if(
            streams_.begin(),
            streams_.end(),
            [&](const AudioStream& current){
                return previous.id == current.id;
            }
        );

        if(it == streams_.end()){
             std::cout 
                << std::left
                << "[-] Node: " << std::setw(5)
                << previous.id
                << previous.application << "/"
                << previous.mediaName << '\n';
        }
    }
    
}

std::vector<AudioStream> AudioMonitor::getActiveStreams() const{
    return streams_;
}

std::optional<StreamId> AudioMonitor::findStreamByApplication(const std::string& application) const{
    for(const auto& stream : streams_){
        if(stream.application == application && stream.isActive && stream.controllable){
            return stream.id;
        }
    }
    return std::nullopt;
}

} // namespace AudioConducker