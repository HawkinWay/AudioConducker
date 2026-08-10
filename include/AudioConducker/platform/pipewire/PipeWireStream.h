#pragma once

#include "AudioConducker/audio/AudioStream.h"
#include "AudioConducker/audio/ActivityDetector.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"

#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>

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
    PipeWireStream(PipeWireContext& context, AudioStream& stream);

    ~PipeWireStream();

    void connect(StreamId id);

    void stop();

private:
    static void on_process(void* userdata);

    void process();
    
    static void on_stream_param_changed(void *_data, uint32_t id, const struct spa_pod *param);

    static const struct pw_stream_events stream_events_;

    PipeWireContext& context_;
    AudioStream& audioStream_;
    struct pw_stream* stream_{nullptr};
    struct spa_audio_info format_{};
    ActivityDetector detector_;
};

} // namespace AudioConducker
