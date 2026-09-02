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
    // context.roundtrip(context.getCore(), context.getMainLoop());

    std::cout << "\n[NodeObserver] Pipewire initialized.\n"; 
}

NodeObserver::~NodeObserver(){
    if(registry_){
        pw_proxy_destroy(reinterpret_cast<pw_proxy*>(registry_));
        registry_ = nullptr;
    }
}

const std::vector<AudioStream>& NodeObserver::getStreams() const{
	return streams_;
}

std::optional<std::reference_wrapper<AudioStream>> NodeObserver::getStreamById(StreamId id){
    for(auto& stream : streams_){
        if(stream.id == id){
            return stream;
        }
    }

    return std::nullopt;
}

pw_registry* NodeObserver::getRegistry() const{
    return registry_;
}

void NodeObserver::setActive(StreamId id, bool active){
    for(auto& stream : streams_){
        if(stream.id == id){
            stream.isActive = active;
            return;
        }
    }
}


// void NodeObserver::setVolume(StreamId id, float volume){
//     auto it = nodes_.find(id);
//     if(it == nodes_.end()){
//         return;
//     }
// 
//     struct pw_node* node = it->second;
//  
//     uint8_t buffer[1024];
//     struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
//     float volumes[2] = {volume, volume};
//     const struct spa_pod* param = reinterpret_cast<const struct spa_pod*>(
//         spa_pod_builder_add_object(
//             &builder, 
//             SPA_TYPE_OBJECT_Props, 
//             SPA_PARAM_Props, 
//             SPA_PROP_channelVolumes, 
//             SPA_POD_Array(sizeof(float), SPA_TYPE_Float, 2, volumes)
//         )
//     );
//     int result = pw_node_set_param(node, SPA_PARAM_Props, 0, param);
//     if(result < 0){
//         std::cerr << "Failed to set volume: " << result << '\n';
//     }
// }

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

    
    std::cout << "\n---- Node found ----\n" << "id: " << id << '\n';
    
    if(props){
        const char* name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const char* app = spa_dict_lookup(props,PW_KEY_APP_NAME);
        const char* media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
	    // const char* media_name = spa_dict_lookup(props, PW_KEY_MEDIA_NAME);
        
        // if(!media_class)    return;
        
        bool isApplication = app != nullptr || (name && strstr(name, "REAPER") != nullptr);
        if(!isApplication)    return;
        
        if(!name)       return;
        
        AudioStream stream = {
            .id = id,
            .name = name ? name : "",
            .application = app ? app : "",
            .mediaClass = media_class ? media_class : "",
	        // .mediaName = media_name ? media_name : ""
        };
        
        std::cout << "name: " << (name ? name : "") << '\n';
        std::cout << "application: " << (app ? app : "") << '\n';
        std::cout << "media class: " << (media_class ? media_class : "") << '\n';
	    // std::cout << "media name: " << (media_name ? media_name : "") << '\n'; 

        observer->streams_.push_back(stream);
        std::cout << "streams now = " << observer->streams_.size() << '\n';
        
        if(observer->callback_ != nullptr){
            observer->callback_(id);
        }
    }
    
}

void NodeObserver::registry_event_global_remove(void *data, uint32_t id){
    auto* observer = static_cast<NodeObserver*>(data);

    spdlog::info("Node removed: {}", id);

    observer->streams_.erase(
        std::remove_if(
            observer->streams_.begin(),
            observer->streams_.end(),
            [id](const AudioStream& audioStream){
                return audioStream.id == id;
            }
        ),
        observer->streams_.end()
    );

    // C++ 20
    // std::erase_if(
    //     observer->streams_, 
    //     [id](const AudioStream& audioStream){
    //         return audioStream.id == id;
    //     }
    // );

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
