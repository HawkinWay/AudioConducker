#include "AudioConducker/platform/pipewire/PipeWireStream.h"

#include <iostream>

namespace AudioConducker{

PipeWireStream::PipeWireStream(PipeWireContext& context, AudioStream& audioStream): context_(context), audioStream_(audioStream){
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
        static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | 
        PW_STREAM_FLAG_MAP_BUFFERS |
        PW_STREAM_FLAG_RT_PROCESS),
        params,
        n_params
    );

    if(result < 0){
        std::cerr << "Failed to connect stream: " << result << '\n';
    }
}

void PipeWireStream::on_process(void* userdata){
    auto pwStream = static_cast<PipeWireStream*>(userdata);
    
    pwStream->process();
}

void PipeWireStream::process(){
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

    static int counter = 0;

    if(counter++ % 50 == 0){
        std::cout << "\nRecieved samples: " << n_samples << '\n';
        bool active = detector_.process(samples, n_samples);
        std::cout << "Active: " << (active ? "true" : "false") << '\n';
    }

    pw_stream_queue_buffer(stream_, pw_buff);
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
    .param_changed = on_stream_param_changed,
    .process = on_process,
};

} // namespace AudioConducker