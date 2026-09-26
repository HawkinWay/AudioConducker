#pragma once

#include <pipewire/pipewire.h>
#include <AudioConducker/core/Logger.h>

#include <condition_variable>
#include <mutex>

namespace AudioConducker{

class PipeWireContext{
public:
    PipeWireContext();

    ~PipeWireContext();

    pw_core* getCore() const;

    pw_main_loop* getMainLoop() const;

    void quit();

    static void roundtrip(struct pw_core* core, struct pw_main_loop* mainLoop);

    void sync();

private:
    struct roundtrip_data {
        int pending;
        struct pw_main_loop *loop;
    };

    static void on_core_done(void *data, uint32_t id, int seq);

    struct sync_data {
        PipeWireContext* self;
        int pending;
        bool done;
        std::mutex mtx;
        std::condition_variable cv;
    };

    static void on_sync_done(void *data, uint32_t id, int seq);

    static int do_sync(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data);

    struct pw_main_loop* loop_;
    struct pw_context* context_;
    struct pw_core* core_;
};

} // namespace AudioConducker
