#include "AudioConducker/pipewire/NodeObserver.h"
#include <iostream>
#include <cstring>

namespace AudioConducker{

NodeObserver::NodeObserver(PipeWireContext& context): registry_(pw_core_get_registry(context.getCore(), PW_VERSION_REGISTRY, 0)){

    spa_zero(listener_);
    
    pw_registry_add_listener(registry_, &listener_, &registry_events_, this);
    // context.roundtrip(context.getCore(), context.getMainLoop());

    std::cout << "\nPipewire initialized.\n"; 
}

NodeObserver::~NodeObserver(){
    pw_proxy_destroy(reinterpret_cast<pw_proxy*>(registry_));
}

const std::vector<AudioStream>& NodeObserver::getStreams() const{
	return streams_;
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
    
    std::cout << "Node found\n" << "id: " << id << '\n';
    
    if(props){
        const char* name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const char* app = spa_dict_lookup(props,PW_KEY_APP_NAME);
        const char* media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);

        if(!media_class)    return;
        if(strcmp(media_class, "Stream/Output/Audio") != 0)     return;

        if(name){
            std::cout << "name: " << name << '\n';
            
            AudioStream stream = {
                .id = id,
                .name = name,
                .application = app ? app : "",
            };
        
            observer->streams_.push_back(stream);
        }
    }
    
}

const struct pw_registry_events NodeObserver::registry_events_ = {
            .version = PW_VERSION_REGISTRY_EVENTS,
            .global = registry_event_global,
};

} // namespace AudioConducker
