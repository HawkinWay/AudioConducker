#pragma once

#include "AudioConducker/core/Logger.h"
#include "AudioConducker/audio/AudioStream.h"
#include "AudioConducker/audio/ActivityDetector.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"

#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>
#include <spa/pod/parser.h>
#include <pipewire/pipewire.h>
#include <functional>

/* Responsibilities:
 *      1. Create PipeWire stream
 *      2. Connect to target node
 *      3. Handle audio callbacks
 *      4. Forward samples to ActivityDetector
 *      5. Handle stream lifecycle
 */

namespace AudioConducker{

class PipeWireStream{
public:
    using ActivityCallback = std::function<void(StreamId, bool)>;

    PipeWireStream(PipeWireContext& context, StreamId id, ActivityCallback activityCallback = nullptr);

    ~PipeWireStream();

    void connect(StreamId id);

    const AudioStream& getAudioStream() const;

private:
    void updateVolumeFromProps(const spa_pod* param);

    static void on_param(void *data, int seq, int32_t id, uint32_t index, uint32_t next, const struct spa_pod *param);

    // void queryVolume();
    
    // void handleProps(const spa_pod* param);

    static void on_process(void* userdata);

    void process();
    
    static void on_state_changed(void *data, pw_stream_state old, pw_stream_state state, const char *error);

    static void on_stream_param_changed(void *_data, uint32_t id, const struct spa_pod *param);

    static const struct pw_stream_events stream_events_;

    StreamId id_;
    PipeWireContext& context_;
    AudioStream audioStream_;

    ActivityCallback activityCallback_;
    bool lastActive_{false};

    struct pw_stream* stream_{nullptr};
    ActivityDetector detector_;
    struct spa_audio_info format_{};
};

} // namespace AudioConducker
