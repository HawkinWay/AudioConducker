#include "AudioConducker/platform/pipewire/PipeWireBackend.h"

namespace AudioConducker{

PipeWireBackend::PipeWireBackend(PipeWireContext& context): context_(context){
    observer_ = std::make_unique<NodeObserver>(
        context, 
        [this](StreamId id){ onNodeAdded(id); },
        [this](StreamId id){ onNodeRemoved(id); } 
    );
}

PipeWireBackend::~PipeWireBackend() = default;

void PipeWireBackend::initialize(){
    context_.roundtrip(context_.getCore(), context_.getMainLoop());
}

void PipeWireBackend::shutdown(){
    spdlog::info("Shutting down PipeWire backend");

    int result = pw_loop_invoke(pw_main_loop_get_loop(context_.getMainLoop()), do_shutdown, 0, nullptr, 0, true, this);

    if(result < 0){
        throw std::runtime_error(
            "Failed to invoke PipeWire backend shutdown"
        );
    }
}


std::vector<AudioStream> PipeWireBackend::getStreams(){
    std::lock_guard<std::mutex> lock(streamMutex_);

    std::vector<AudioStream> result;

    result.reserve(streams_.size());

    for(const auto& stream : streams_){
        result.push_back(stream.second);
    }

    return result;
}

void PipeWireBackend::setVolume(StreamId id, float volume){
    
    struct SetVolumeData data = {
        .self = this,
        .id = id,
        .volume = volume,
    };


    int result = pw_loop_invoke(pw_main_loop_get_loop(context_.getMainLoop()), do_set_volume, 0, &data, sizeof(data), 1, nullptr);

}

void PipeWireBackend::setVolumeInternal(StreamId id, float volume){
    spdlog::trace(
        "[setVolumeInternal] node={} volume={}",
        id,
        volume
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


int PipeWireBackend::do_shutdown(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data){

    auto* self = static_cast<PipeWireBackend*>(user_data);

    spdlog::info("Destroying PipeWire backend resources...");

    self->monitors_.clear();

    for(auto& [id, nodeData] : self->node_data_){
        spa_hook_remove(&nodeData->node_listener);
    }

    self->node_data_.clear();

    for(auto& [id, node] : self->nodes_){
        if(node){
            pw_proxy_destroy(reinterpret_cast<pw_proxy*>(node));
        }
    }

    self->nodes_.clear();
    self->streams_.clear();

    self->observer_.reset();

    spdlog::info("PipeWire backend shutdown complete");

    return 0;
}


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

void PipeWireBackend::onNodeInfo(void *data, const struct pw_node_info *info){
    auto* nodeData = static_cast<NodeData*>(data);
    
    auto* backend = nodeData->backend;
    
    if(!info || !info->props){
        return;
    }
    
    const auto* props = info->props;


    if(const char* node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME))        nodeData->nodeName = node_name;
    if(const char* application = spa_dict_lookup(props, PW_KEY_APP_NAME))       nodeData->application = application;
    if(const char* media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS))    nodeData->mediaClass = media_class;
    if(const char* media_name = spa_dict_lookup(props, PW_KEY_MEDIA_NAME))      nodeData->mediaName = media_name;

    if(nodeData->mediaClass != "Stream/Output/Audio") {
        return;
    }

    if(nodeData->mediaName.empty()){
        return;
    }
    
    std::lock_guard<std::mutex> lock(backend->streamMutex_);

    auto& stream = backend->streams_[nodeData->id];
    
    stream.id = nodeData->id;
    stream.name = nodeData->nodeName;
    stream.application = nodeData->application;
    stream.mediaClass = nodeData->mediaClass;
    stream.mediaName = nodeData->mediaName;
    stream.controllable = true;

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

        const float* volumes = static_cast<const float*>(SPA_POD_ARRAY_VALUES(array));
        // const uint32_t count = array->body.child.size / sizeof(float);
        const uint32_t count = SPA_POD_ARRAY_N_VALUES(array);

        if(count == 0)  continue;

        float sum = 0.f;

        for(uint32_t i = 0; i < count; i++){
            sum += volumes[i];
        }

        const float average = sum / static_cast<float>(count);

        std::lock_guard<std::mutex> lock(streamMutex_);
        auto it = streams_.find(streamId);
        if(it != streams_.end())    it->second.volume = average;

        spdlog::trace(
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
    nodeData->node = node;

    static const pw_node_events node_events = {
        .version = PW_VERSION_NODE_EVENTS,
        .info = onNodeInfo,
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

    queryVolume(id);

    auto monitor = std::make_unique<PipeWireStream>(
        context_,
        id,
        [this](StreamId id, bool active){
            onActivityChanged(id, active);
        }
    );

    monitor->connect(id);

    monitors_[id] = std::move(monitor);

    spdlog::debug("Monitoring node {}", id);
}

void PipeWireBackend::onNodeRemoved(StreamId id){
    spdlog::debug("[PipeWireBackend] onNodeRemoved {}", id);

    auto it = nodes_.find(id);

    if(it != nodes_.end()){
        if(it->second){
            pw_proxy_destroy(
                reinterpret_cast<pw_proxy*>(it->second)
            );
        }

        nodes_.erase(it);
    }

    monitors_.erase(id);
    node_data_.erase(id);
    {
        std::lock_guard<std::mutex> lock(streamMutex_);
        streams_.erase(id);
    }
}

void PipeWireBackend::onActivityChanged(StreamId id, bool active){
    auto it = streams_.find(id);
    if(it == streams_.end())    return;

    it->second.isActive = active;
}

} // namespace AudioConducker
