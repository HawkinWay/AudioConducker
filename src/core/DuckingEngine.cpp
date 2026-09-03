#include "AudioConducker/core/DuckingEngine.h"

#include <iostream>

namespace AudioConducker{

DuckingEngine::DuckingEngine(AudioBackend& backend, float duckLevel): backend_(backend), duckLevel_(duckLevel){}

void DuckingEngine::process(std::optional<StreamId> focusStream){
    Logger::info("DuckingEngine processing...\n");
    auto streams = backend_.getStreams();

    if(!focusStream){
        spdlog::info("Focus stream not found -> restore");
        restore();
        return;
    }

    bool focusActive = false;

    for(const auto &stream : streams){
        if(stream.id == *focusStream){
            focusActive = stream.isActive;
            break;
        }
    }

    if(focusActive){
        duck(*focusStream, streams);
    }else{
        restore();
    }
}

void DuckingEngine::restore(){
    if(!isActive_){
        return;
    }

    spdlog::info("1111111111RESTORE called.");

    for(const auto& oV : originalVolumes_){     
        backend_.setVolume(oV.first, oV.second);
    }

    originalVolumes_.clear();

    isActive_ = false;
}

void DuckingEngine::duck(StreamId focusStream, const std::vector<AudioStream>& streams){
    if(isActive_){
        return;
    }

    originalVolumes_.clear();

    for(const auto &stream : streams){
        if(stream.id == focusStream)    continue;
        if(!stream.controllable)        continue;

        spdlog::info(
            "DUCK: focus={} stream={} volume={}",
            focusStream,
            stream.id,
            stream.volume
        );
        
        originalVolumes_[stream.id] = stream.volume;

        backend_.setVolume(stream.id, stream.volume * duckLevel_);
    }

    isActive_ = true;
}


} // namespace AudioConducker