#include "AudioConducker/core/DuckingEngine.h"


namespace AudioConducker{

DuckingEngine::DuckingEngine(IAudioBackend& backend, float duckAmount, float attackTime, float releaseTime, float holdTime): 
        backend_(backend), 
        duckAmount_(duckAmount),
        attackTime_(attackTime),
        releaseTime_(releaseTime),
        // holdTime_(std::chrono::milliseconds(int(holdTime * 1000)))
        holdTime_(holdTime){}

void DuckingEngine::process(std::optional<StreamId> focusStream, float deltaTime){

    auto streams = backend_.getStreams();

    bool focusActive = false;

    if(focusStream){
        for(const auto &stream : streams){
            if(stream.id == *focusStream){
                focusActive = stream.isActive;
                break;
            }
        }
    }
    
    if(focusActive){
        // lastTime_ = Clock::now();
        sinceLastActive_ = 0.f;
        updateDucking(*focusStream, streams, deltaTime);
        
    }else{
        // if(!lastTime_ || Clock::now() - *lastTime_ < holdTime_){
            
        // }
        sinceLastActive_ += deltaTime;
        if(sinceLastActive_ >= holdTime_){
            updateRestore(deltaTime);
        }
        else if(!states_.empty()){
            for(auto& [id, state] : states_){
                if(!state.smoother.isSmoothing())    continue;
                backend_.setVolume(id, state.smoother.process(deltaTime));
            }
        }
    }

    syncWithStreams(focusStream, streams);

}

void DuckingEngine::shutDown(){
    spdlog::info("xxxxx SHUT DOWN called xxxxx");
    
    for(const auto& [id, state] : states_){
        backend_.setVolume(id, state.originalVolume);
    }

    states_.clear();
}

void DuckingEngine::updateDucking(std::optional<StreamId> focusStream, const std::vector<AudioStream>& streams, float deltaTime){
    for(const auto& stream : streams){
        if(stream.id == *focusStream)   continue;
        if(!stream.controllable) continue;
        
        auto it = states_.find(stream.id);
        
        if(it == states_.end()){
            StreamState state = {
                .originalVolume = stream.volume,
                .smoother = VolumeSmoother(attackTime_, releaseTime_),
            };
            
            state.smoother.setCurrent(stream.volume);
            
            it = states_.emplace(stream.id, std::move(state)).first;
        }
        
        auto& state = it->second;
        
        float target = state.originalVolume * (1 - duckAmount_);
        state.smoother.setTarget(target);
        
        float volume = state.smoother.process(deltaTime);
        
        backend_.setVolume(stream.id, volume);
    }
}

void DuckingEngine::updateRestore(float deltaTime){
    for(auto it = states_.begin(); it != states_.end(); ){
        auto& state = it->second;
        state.smoother.setTarget(state.originalVolume);
        float volume = state.smoother.process(deltaTime);
        backend_.setVolume(it->first, volume);

        if(!state.smoother.isSmoothing()){
            it = states_.erase(it);
        }else{
            it++;
        }
    }
}

void DuckingEngine::syncWithStreams(std::optional<StreamId> focusStream, const std::vector<AudioStream>& streams){
    std::erase_if(states_,
                  [&](const auto& kv){
                    return !std::any_of(streams.begin(), 
                                        streams.end(), 
                                        [&](const AudioStream& sm){
                                            return sm.id == kv.first;
                                        });
                  });
    
    if(!focusStream)    return;

    for(const auto& stream : streams){
        if(stream.id == focusStream) continue;
        if(!stream.controllable)    continue;
        if(states_.contains(stream.id)) continue;

        StreamState streamState = {
            .originalVolume = stream.volume,
            .smoother = VolumeSmoother(attackTime_, releaseTime_),
        };
        streamState.smoother.setCurrent(stream.volume);
        states_.emplace(stream.id, std::move(streamState));
    }
}

} // namespace AudioConducker