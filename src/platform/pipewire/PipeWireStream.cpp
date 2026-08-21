#include "AudioConducker/platform/pipewire/PipeWireStream.h"

#include <iostream>

namespace AudioConducker{

PipeWireStream::PipeWireStream(PipeWireContext& context, StreamId id, ActivityCallback activityCallback): 
                                context_(context), id_(id), activityCallback_(std::move(activityCallback)){
    struct pw_properties* props = pw_properties_new(
                                    PW_KEY_MEDIA_TYPE, "Audio",
                                    PW_KEY_MEDIA_CATEGORY, "Capture",
                                    PW_KEY_MEDIA_ROLE, "Music",
                                    PW_KEY_NODE_NAME, "AudioConducker-monitor",
                                    nullptr
                                );


    stream_ = pw_stream_new_simple(
                pw_main_loop_get_loop(context_.getMainLoop()),
                "AudioConducker-stream",
                props,
                &stream_events_,
                this
            );

}

PipeWireStream::~PipeWireStream(){
    if(stream_){
        pw_stream_destroy(stream_);
        stream_ = nullptr;
    }
}

void PipeWireStream::connect(StreamId id){
    const struct spa_pod *params[1];
    uint32_t n_params = 0; 
    uint8_t buffer[1024];
    struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const struct spa_audio_info_raw info = SPA_AUDIO_INFO_RAW_INIT( .format = SPA_AUDIO_FORMAT_F32 );

    params[n_params++] = spa_format_audio_raw_build(
                            &builder, 
                            SPA_PARAM_EnumFormat, 
                            &info
                        );
    
    int result = pw_stream_connect(
        stream_, 
        PW_DIRECTION_INPUT, 
        id, 
        static_cast<pw_stream_flags>(
            PW_STREAM_FLAG_AUTOCONNECT | 
            PW_STREAM_FLAG_MAP_BUFFERS |
            PW_STREAM_FLAG_RT_PROCESS
        ),
        params,
        n_params
    );

    std::cout << "[PipeWireStream] connect result = " << result << '\n';
    std::cout << "[PipeWireStream] stream state = " << pw_stream_state_as_string(pw_stream_get_state(stream_, nullptr)) << '\n';

    if(result < 0){
        std::cerr << "Failed to connect stream: " << result << '\n';
    }
}

void PipeWireStream::on_process(void* userdata){
    auto pwStream = static_cast<PipeWireStream*>(userdata);
    
    pwStream->process();
}

void PipeWireStream::process(){
    spdlog::info(
        "process() thread {}",
        std::hash<std::thread::id>{}(std::this_thread::get_id())
    );

    struct pw_buffer* pw_buff;
    struct spa_buffer* spa_buff;
    float *samples;
    uint32_t n_channels, n_samples;

    if((pw_buff = pw_stream_dequeue_buffer(stream_)) == nullptr){
        std::cerr << "No buffer available\n";
        return;
    }
    spa_buff = pw_buff->buffer;
    if((samples = static_cast<float*>(spa_buff->datas[0].data)) == nullptr){
        return;
    }

    n_channels = format_.info.raw.channels;
    n_samples = spa_buff->datas[0].chunk->size / sizeof(float);

    bool active = detector_.process(samples, n_samples);
    // spdlog::info(
    //     "PipeWireStream {}: this={}, rms={}, active={}",
    //     audioStream_.id,
    //     static_cast<const void*>(this),
    //     detector_.getRMS(samples, n_samples),
    //     active
    // );
    if(active != lastActive_){
        lastActive_ = active;
        
        if(activityCallback_){
            activityCallback_(id_, active);
        }
    }

    
    static uint64_t processCount = 0;
    processCount++;
    if (processCount % 100 == 0) {
        spdlog::debug(
            "PipeWireStream {} processed {} buffers, rms = {} active = {}",
            id_,
            processCount,
            detector_.getRMS(samples, n_samples),
            (active ? "yes" : "no")
        );
    }

    pw_stream_queue_buffer(stream_, pw_buff);
}

void PipeWireStream::on_state_changed(void *data, pw_stream_state old, pw_stream_state state, const char *error){
    std::cout << "Stream state: " << pw_stream_state_as_string(state) << '\n';

    if (error)
        std::cerr << "Error: " << error << '\n';
}


void PipeWireStream::on_stream_param_changed(void *_data, uint32_t id, const struct spa_pod *param){
    auto pwStream = static_cast<PipeWireStream*>(_data);

    if(param == nullptr){
        return;
    }

    if(spa_format_parse(param, &pwStream->format_.media_type, &pwStream->format_.media_subtype) < 0){
        return;
    }

    if(pwStream->format_.media_type != SPA_MEDIA_TYPE_audio || pwStream->format_.media_subtype != SPA_MEDIA_SUBTYPE_raw){
        return;
    }

    spa_format_audio_raw_parse(param, &pwStream->format_.info.raw);
    std::cout << "\nCapturing rate: " << pwStream->format_.info.raw.rate << "\nChannels: " << pwStream->format_.info.raw.channels << '\n';
}


const struct pw_stream_events PipeWireStream::stream_events_ = {
    .version = PW_VERSION_STREAM_EVENTS,
    .state_changed = on_state_changed,
    .param_changed = on_stream_param_changed,
    .process = on_process,
};

} // namespace AudioConducker