#include "../../include/AudioConducker/core/DuckingEngine.h"

namespace AudioConducker{

DuckingEngine::DuckingEngine(AudioBackend& backend): backend_(backend){}

void DuckingEngine::process(StreamId focusStream){
    auto streams = backend_.getStreams();

    for(auto &stream : streams){
        if(stream.id != focusStream){
            backend_.setVolume(stream.id, duckVolume_);
        }
    }
}

} // namespace AudioConducker