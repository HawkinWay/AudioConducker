#include "AudioConducker/pipewire/PipeWireContext.h"
#include <stdexcept>


namespace AudioConducker{

PipeWireContext::PipeWireContext(){
    pw_init(nullptr, nullptr);

    loop_ = pw_main_loop_new(nullptr);
    if(!loop_){
        throw std::runtime_error("Failed to create Pipewire main loop");
    }

    context_ = pw_context_new(pw_main_loop_get_loop(loop_), nullptr, 0);
    if(!context_){
        throw std::runtime_error("Failed to create Pipewire context");
    }

    core_ = pw_context_connect(context_, nullptr, 0);
    if(!core_){
        throw std::runtime_error("Failed to connect to Pipewire core");
    }
}

PipeWireContext::~PipeWireContext(){
    if(core_){
        pw_core_disconnect(core_);
    }
    
    if(context_){
        pw_context_destroy(context_);
    }
    
    if(loop_){
        pw_main_loop_destroy(loop_);
    }

    pw_deinit();
}

pw_core* PipeWireContext::getCore() const{
    return core_;
}

} // namespace AudioConducker