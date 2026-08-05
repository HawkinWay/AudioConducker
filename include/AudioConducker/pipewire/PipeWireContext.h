#pragma once

#include <pipewire/pipewire.h>

namespace AudioConducker{

class PipeWireContext{
public:
    PipeWireContext();

    ~PipeWireContext();

    pw_core* getCore() const;

    pw_main_loop* getMainLoop() const;

	// static void roundtrip(struct pw_core* core, struct pw_main_loop* mainLoop);

private:
    // struct roundtrip_data {
    //     int pending;
    //     struct pw_main_loop *loop;
    // };

    // static void on_core_done(void *data, uint32_t id, int seq);

    struct pw_main_loop* loop_;
    struct pw_context* context_;
    struct pw_core* core_;
};

} // namespace AudioConducker
