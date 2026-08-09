#include "AudioConducker/platform/pipewire/PipeWireBackend.h"

namespace AudioConducker{

PipeWireBackend::PipeWireBackend(PipeWireContext& context): context_(context), 
                                                            observer_(std::make_unique<NodeObserver>(
                                                                context, 
                                                                [this](StreamId id){
                                                                    onNodeAdded(id);
                                                                }
                                                            )){
    
}

PipeWireBackend::~PipeWireBackend(){
    for(auto& node : nodes_){
        pw_proxy_destroy(reinterpret_cast<struct pw_proxy*>(node.second));
    }
}

std::vector<AudioStream> PipeWireBackend::getStreams(){
    return observer_->getStreams();
}

void PipeWireBackend::setVolume(StreamId id, float volume){
    auto it = nodes_.find(id);
    if(it == nodes_.end()){
        return;
    }

    struct pw_node* node = it->second;
    
    uint8_t buffer[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    float volumes[2] = {volume, volume};
    const struct spa_pod* param = reinterpret_cast<const struct spa_pod*>(
        spa_pod_builder_add_object(
            &builder, 
            SPA_TYPE_OBJECT_Props, 
            SPA_PARAM_Props, 
            SPA_PROP_channelVolumes, 
            SPA_POD_Array(sizeof(float), SPA_TYPE_Float, 2, volumes)
        )
    );
    int result = pw_node_set_param(node, SPA_PARAM_Props, 0, param);
    if(result < 0){
        std::cerr << "Failed to set volume: " << result << '\n';
    }
}

void PipeWireBackend::onNodeAdded(StreamId id){
    auto node = reinterpret_cast<pw_node*>(
        pw_registry_bind(observer_->getRegistry(), id, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0)
    );	
    nodes_[id] = node;

    std::cout << "Backend bound node: " << id << '\n';

    // test
    // if(id == 84){
    //     std::cout << "set volume 0.2f for node: " << id << '\n'; 
    //     setVolume(id, 0.2f);
    // }
}

} // namespace AudioConducker
