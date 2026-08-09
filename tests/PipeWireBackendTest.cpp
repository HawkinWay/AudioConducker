#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include "AudioConducker/platform/pipewire/PipeWireBackend.h"

using namespace AudioConducker;

TEST(PipeWireBackendTest, DiscoverStreams){
    PipeWireContext context;
    PipeWireBackend backend(context);

    std::thread loop([&](){
        pw_main_loop_run(context.getMainLoop());
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto streams = backend.getStreams();

    EXPECT_GT(streams.size(), 0);

    pw_main_loop_quit(context.getMainLoop());

    loop.join();
}

TEST(PipeWireBackendTest, SetVolume){
    PipeWireContext context;
    PipeWireBackend backend(context);

    std::thread loop([&]() {
            pw_main_loop_run(context.getMainLoop());
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto streams = backend.getStreams();

    ASSERT_FALSE(streams.empty());

    auto id = streams[0].id;

    backend.setVolume(id, 0.2f);

    pw_main_loop_quit(context.getMainLoop());

    loop.join();
}