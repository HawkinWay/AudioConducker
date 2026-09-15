#include "AudioConducker/core/DuckingEngine.h"

#include <iostream>

namespace AudioConducker{

DuckingEngine::DuckingEngine(AudioBackend& backend, float duckAmount): backend_(backend), duckAmount_(duckAmount){}

void DuckingEngine::process(std::optional<StreamId> focusStream){
    // Logger::info("DuckingEngine processing...\n");
    auto streams = backend_.getStreams();

    if(!focusStream){
        spdlog::debug("Focus stream not found -> restore");
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

void DuckingEngine::shutDown(){
    spdlog::info("xxxSHUT DOWN calledxxx");
    restore();
}

void DuckingEngine::restore(){
    if(!isActive_){
        return;
    }

    
    for(const auto& oV : originalVolumes_){     
        backend_.setVolume(oV.first, oV.second);
        spdlog::debug(
            "Restoring stream {} to original volume {}",
            oV.first,
            oV.second
        );
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

        // spdlog::info(
        //     "DUCK: focus={} stream={} volume={}",
        //     focusStream,
        //     stream.id,
        //     stream.volume
        // );
        
        originalVolumes_[stream.id] = stream.volume;

        backend_.setVolume(stream.id, stream.volume * (1.f - duckAmount_));
    }

    isActive_ = true;
}


} // namespace AudioConducker