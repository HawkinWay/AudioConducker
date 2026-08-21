#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include <stdexcept>
#include <iostream>


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

pw_main_loop* PipeWireContext::getMainLoop() const{
    return loop_;
}


void PipeWireContext::roundtrip(struct pw_core* core, struct pw_main_loop* mainLoop){
    static const struct pw_core_events core_events = {
        .version = PW_VERSION_CORE_EVENTS,
        .done = on_core_done,
    };

    struct roundtrip_data d = { .loop = mainLoop, };
    struct spa_hook core_listener;
    int err;

    pw_core_add_listener(core, &core_listener, &core_events, &d);
    
    d.pending = pw_core_sync(core, PW_ID_CORE, 0);
    if((err = pw_main_loop_run(mainLoop)) < 0){
        std::cout << "main_loop_run error: " << err << '\n';
    }

    spa_hook_remove(&core_listener);
}


void PipeWireContext::on_core_done(void *data, uint32_t id, int seq)
{
    struct roundtrip_data *d = reinterpret_cast<roundtrip_data*>(data);
    
    if (id == PW_ID_CORE && seq == d->pending){
        pw_main_loop_quit(d->loop);
    }
}

} // namespace AudioConducker