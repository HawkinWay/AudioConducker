#include "AudioConducker/core/AudioMonitor.h"

#include <iostream>
#include <iomanip>

namespace AudioConducker{

AudioMonitor::AudioMonitor(AudioBackend& backend) : backend_(backend){}

void AudioMonitor::update(){
    previousStreams_ = streams_;
    streams_ = backend_.getStreams();
}

void AudioMonitor::showNodes(){
    std::cout << std::string(55, '=') << " Nodes " << std::string(55, '=') << '\n';

    std::cout 
            << std::left 
            << std::setw(8)  << "ID" 
            << std::setw(30) << "Name" 
            << std::setw(30) << "Application" 
            << std::setw(30) << "Media Class" 
            << std::setw(20) << "Media Name" << '\n';
    std::cout << std::string(116, '-') << '\n';
    
    
    update();
    
    std::vector<AudioStream> streams = getActiveStreams();
    for(const auto& stream : streams){
        std::cout 
                << std::left 
                << std::setw(8) << stream.id 
                << std::setw(30) << stream.name 
                << std::setw(30) << stream.application 
                << std::setw(30) << stream.mediaClass 
                << std::setw(20) << stream.mediaName << '\n';
    }

    std::cout << std::string(116, '=') << '\n';
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