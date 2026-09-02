#include "AudioConducker/core/DuckingEngine.h"

#include <iostream>

namespace AudioConducker{

DuckingEngine::DuckingEngine(AudioBackend& backend, float duckLevel): backend_(backend), duckLevel_(duckLevel), currentFocusStream_(0){}

void DuckingEngine::process(StreamId focusStream){
    Logger::info("DuckingEngine processing...\n");
    auto streams = backend_.getStreams();

    bool focusActive = false;

    for(const auto &stream : streams){
        if(stream.id == focusStream){
            focusActive = stream.isActive;
            break;
        }
    }

    if(isActive_ && currentFocusStream_ != focusStream){
        restore();
    }

    if(focusActive){
        duck(focusStream, streams);
    }else{
        restore();
    }
}

void DuckingEngine::restore(){
    if(!isActive_){
        return;
    }

    for(const auto& oV : originalVolumes_){
        backend_.setVolume(oV.first, oV.second);
    }

    originalVolumes_.clear();

    isActive_ = false;

    currentFocusStream_ = 0;
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