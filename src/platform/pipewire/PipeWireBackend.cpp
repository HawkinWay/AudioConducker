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
    auto streams = observer_->getStreams();

    for(auto& stream : streams){
        auto it = volumes_.find(stream.id);

        if(it != volumes_.end()){
            stream.volume = it->second;
        }
    }

    return streams;
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

#if 0
void PipeWireBackend::updateVolumeFromProps(const spa_pod* param){
    if (!spa_pod_is_object(param)) {
        return;
    }

    const auto* object = reinterpret_cast<const spa_pod_object*>(param);
    const spa_pod_prop* prop = nullptr;

    SPA_POD_OBJECT_FOREACH(object, prop){
        if(prop->key != SPA_PROP_channelVolumes) {
            continue;
        }

        const spa_pod* value = &prop->value;

        if (!spa_pod_is_array(value)) {
            continue;
        }

        const auto* array = reinterpret_cast<const spa_pod_array*>(value);
        if(array->body.child.type != SPA_TYPE_Float){
            continue;
        }

        const float* volumes = static_cast<const float*>(SPA_POD_BODY(&array->body));
        const uint32_t count = array->body.child.size / sizeof(float);

        if(count == 0)  continue;

        float sum = 0.f;

        for(uint32_t i = 0; i < count; i++){
            sum += volumes[i];
        }

        const float average = sum / static_cast<float>(count);

        audioStream_.volume = average;

        spdlog::info(
            "PipeWireStream {} volume updated: {}",
            audioStream_.id,
            audioStream_.volume
        );

        break;

    }

    spa_pod_parser parser;
    spa_pod_parser_init(&parser, nullptr, 0);

    uint32_t prop;
    uint32_t flags;

    const float* volumes = nullptr;
    uint32_t n_volume = 0;

    int res;

    while((res = spa_pod_parser_get(&parser, param, SPA_TYPE_Object, nullptr)) >= 0){

    }
}

void PipeWireBackend::on_param(void *data, int seq, int32_t id, uint32_t index, uint32_t next, const struct spa_pod *param){
    auto *self = static_cast<PipeWireBackend*>(data);
    if(param == nullptr)    return;

    if(id == SPA_PARAM_Props){
        self->updateVolumeFromProps(param);
    }
}
#endif

void PipeWireBackend::queryVolume(StreamId id){
    struct QueryVolumeData data = {
        .self = this,
        .id = id
    };

    pw_loop_invoke(pw_main_loop_get_loop(context_.getMainLoop()), do_query_volume, 0, &data, sizeof(data), 0, nullptr);
}

int PipeWireBackend::do_query_volume(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data){
    const auto* qd = static_cast<const QueryVolumeData*>(data);

    auto it = qd->self->nodes_.find(qd->id);
    if(it == qd->self->nodes_.end())
        return 0;

    pw_node_enum_params(it->second, 0, SPA_PARAM_Props, 0, 1, nullptr);

    return 0;
}

void PipeWireBackend::onNodeParam(void *data, int seq, uint32_t id, uint32_t index, uint32_t next, const struct spa_pod *param){
    auto* nodeData = static_cast<NodeData*>(data);

    nodeData->backend->handleNodeProps(nodeData->id, id, param);
}

void PipeWireBackend::handleNodeProps(StreamId streamId, uint32_t id, const spa_pod* param){
     if (!spa_pod_is_object(param)) {
        return;
    }

    const auto* object = reinterpret_cast<const spa_pod_object*>(param);
    const spa_pod_prop* prop = nullptr;

    SPA_POD_OBJECT_FOREACH(object, prop){
        if(prop->key != SPA_PROP_channelVolumes) {
            continue;
        }

        const spa_pod* value = &prop->value;

        if (!spa_pod_is_array(value)) {
            continue;
        }

        const auto* array = reinterpret_cast<const spa_pod_array*>(value);
        if(array->body.child.type != SPA_TYPE_Float){
            continue;
        }

        const float* volumes = static_cast<const float*>(SPA_POD_BODY(&array->body));
        const uint32_t count = array->body.child.size / sizeof(float);

        if(count == 0)  continue;

        float sum = 0.f;

        for(uint32_t i = 0; i < count; i++){
            sum += volumes[i];
        }

        const float average = sum / static_cast<float>(count);

       volumes_[streamId] = average;

        spdlog::info(
            "Node {} volume = {}",
            streamId,
            average
        );

        return;

    }
}



void PipeWireBackend::onNodeAdded(StreamId id){
    auto node = reinterpret_cast<pw_node*>(
        pw_registry_bind(
            observer_->getRegistry(),
            id,
            PW_TYPE_INTERFACE_Node,
            PW_VERSION_NODE,
            0
        )
    );

    if(!node)   return;

    nodes_[id] = node;

    auto nodeData = std::make_unique<NodeData>();

    nodeData->backend = this;
    nodeData->id = id;

    static const pw_node_events node_events = {
        .version = PW_VERSION_NODE_EVENTS,
        .param = onNodeParam,
    };

    spa_zero(nodeData->node_listener);

    pw_node_add_listener(
        node,
        &nodeData->node_listener,
        &node_events,
        nodeData.get()
    );

    node_data_[id] = std::move(nodeData);

    // 查询真实 volume
    queryVolume(id);

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
