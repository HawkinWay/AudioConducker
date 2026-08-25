#include "AudioConducker/platform/pipewire/PipeWireBackend.h"

namespace AudioConducker{

PipeWireBackend::PipeWireBackend(PipeWireContext& context): context_(context){
    observer_ = std::make_unique<NodeObserver>(context, [this](StreamId id){ onNodeAdded(id); });
}

PipeWireBackend::~PipeWireBackend(){
    for(auto& node : nodes_){
        struct DestroyProxyData data = {
            .proxy = reinterpret_cast<struct pw_proxy*>(node.second),
        };
        pw_loop_invoke(pw_main_loop_get_loop(context_.getMainLoop()), do_destroy_proxy, 0, &data, sizeof(data), 0, nullptr);
    }
}

void PipeWireBackend::initialize(){
    context_.roundtrip(context_.getCore(), context_.getMainLoop());
}

std::vector<AudioStream> PipeWireBackend::getStreams(){
    // std::cout << "\ngetStreams(): size: " << observer_->getStreams().size() << '\n';
    // for (const auto& stream : observer_->getStreams()) {
    //     spdlog::info(
    //         "Backend stream {}: active={}",
    //         stream.id,
    //         stream.isActive
    //     );
    // }
    return observer_->getStreams();
}

void PipeWireBackend::setVolume(StreamId id, float volume){
    auto it = nodes_.find(id);
    if(it == nodes_.end())  return;
    
    struct SetVolumeData data = {
        .self = this,
        .id = id,
        .volume = volume,
    };

    pw_loop_invoke(pw_main_loop_get_loop(context_.getMainLoop()), do_set_volume, 0, &data, sizeof(data), 0, nullptr);
}

void PipeWireBackend::setVolumeInternal(StreamId id, float volume){
    spdlog::info(
        "setVolume({}) called from thread {}",
        id,
        std::hash<std::thread::id>{}(std::this_thread::get_id())
    );

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

int PipeWireBackend::do_set_volume(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data){
    const auto* vd = static_cast<const SetVolumeData*>(data);
    vd->self->setVolumeInternal(vd->id, vd->volume);
    return 0;
}

int PipeWireBackend::do_destroy_proxy(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data){
    const auto* pd = static_cast<const DestroyProxyData*>(data);
    if(pd->proxy){
        pw_proxy_destroy(pd->proxy);
    }
    return 0;
}

void PipeWireBackend::onNodeAdded(StreamId id){
    auto node = reinterpret_cast<pw_node*>(
        pw_registry_bind(observer_->getRegistry(), id, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0)
    );	
    nodes_[id] = node;

    auto stream = observer_->getStreamById(id);

    if(!stream){
        spdlog::error("Stream {} not found", id);
        return;
    }
    
    auto monitor = std::make_unique<PipeWireStream>(
        context_, 
        id, 
        [this](StreamId id, bool active){
            onActivityChanged(id, active);
        }
    );

    monitor->connect(id);

    monitors_[id] = std::move(monitor);
    
    spdlog::info("Monitoring node {}", id);
}

void PipeWireBackend::onActivityChanged(StreamId id, bool active){
    observer_->setActive(id, active);
}

} // namespace AudioConducker
