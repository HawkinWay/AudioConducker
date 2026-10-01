#include "AudioConducker/platform/pipewire/NodeObserver.h"
#include <iostream>
#include <cstring>

namespace AudioConducker{

NodeObserver::NodeObserver(PipeWireContext& context, NodeCallback callback, NodeRemovedCallback removedCallback): 
        registry_(pw_core_get_registry(context.getCore(), PW_VERSION_REGISTRY, 0)),
        callback_(std::move(callback)),
        removedCallback_(std::move(removedCallback)){

    spa_zero(listener_);
    
    pw_registry_add_listener(registry_, &listener_, &registry_events_, this);
}

NodeObserver::~NodeObserver(){
    if(registry_){
        pw_proxy_destroy(reinterpret_cast<pw_proxy*>(registry_));
        registry_ = nullptr;
    }
}


pw_registry* NodeObserver::getRegistry() const{
    return registry_;
}


void NodeObserver::registry_event_global(
    void *data, 
    uint32_t id,
    uint32_t permissions, 
    const char *type, 
    uint32_t version,
    const struct spa_dict *props
){
    if(strcmp(type, PW_TYPE_INTERFACE_Node) != 0){
        return;
    }
    
    auto observer = static_cast<NodeObserver*>(data);

    
    spdlog::debug("---- Node found ----");
    
    if(props){
        const struct spa_dict_item* item;


        const char* name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const char* app = spa_dict_lookup(props,PW_KEY_APP_NAME);
        const char* media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
       
        if(!media_class)    return;
        
        bool isApplication = app != nullptr || (name && strstr(name, "REAPER") != nullptr);
        if(!isApplication)    return;
        
        if(!name)       return;

        if(strcmp(media_class, "Stream/Output/Audio") != 0)  return;
        
  
        spdlog::debug(
            "Node discovered: id={} \nname={} \napp={} \nmediaClass={}",
            id,
            name,
            app ? app : "",
            media_class ? media_class : ""
        );


        if(observer->callback_ != nullptr){
            observer->callback_(id);
        }
    }
    
}

void NodeObserver::registry_event_global_remove(void *data, uint32_t id){
    auto* observer = static_cast<NodeObserver*>(data);

    spdlog::debug("[NodeObserver] registry_event_global_remove: {}", id);

    if(observer->removedCallback_ != nullptr){
        observer->removedCallback_(id);
    }
}

const struct pw_registry_events NodeObserver::registry_events_ = {
        .version = PW_VERSION_REGISTRY_EVENTS,
        .global = registry_event_global,
        .global_remove = registry_event_global_remove,
};

} // namespace AudioConducker
