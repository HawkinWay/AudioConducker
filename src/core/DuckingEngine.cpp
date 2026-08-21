#include "AudioConducker/core/DuckingEngine.h"

#include <iostream>

namespace AudioConducker{

DuckingEngine::DuckingEngine(AudioBackend& backend, float duckLevel): backend_(backend), duckLevel_(duckLevel){}

void DuckingEngine::process(StreamId focusStream){
    std::cout << "DuckingEngine processing...\n";
    auto streams = backend_.getStreams();

    bool focusActive = false;

    for(const auto &stream : streams){
        if(stream.id == focusStream){
            focusActive = stream.isActive;
            break;
        }
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
}

void DuckingEngine::duck(StreamId focusStream, const std::vector<AudioStream>& streams){
    if(isActive_){
        return;
    }

    for(auto &stream : streams){
        if(stream.id == focusStream)    continue;
        if(!stream.controllable)        continue;

        originalVolumes_[stream.id] = stream.volume;

        backend_.setVolume(stream.id, stream.volume * duckLevel_);
    }

    isActive_ = true;
}


} // namespace AudioConducker