#include "AudioConducker/audio/ActivityDetector.h"
#include "AudioConducker/core/DuckingEngine.h"
#include "AudioConducker/core/ConfigManager.h"
#include "AudioConducker/core/AudioMonitor.h"
#include "AudioConducker/core/Logger.h"
#include "AudioConducker/platform/pipewire/NodeObserver.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include "AudioConducker/platform/pipewire/PipeWireBackend.h"
#include "AudioConducker/platform/pipewire/PipeWireStream.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace AudioConducker;

int main()
{
    // test logger
    AudioConducker::Logger::init();

    AudioConducker::Logger::info("Application started");

    AudioConducker::Logger::debug("Debug message");

    AudioConducker::Logger::error("Example error\n");

    //test pipewire
    try{
#if 0
        AudioConducker::PipeWireContext context;
        std::cout << "Pipewire context initialized.\n";

        AudioConducker::AudioStream firefox{
            .id = 114,
            .name = "Firefox",
        };

        AudioConducker::PipeWireStream stream(context, firefox);

        stream.connect(114);

        
        AudioConducker::PipeWireBackend backend(context);
        std::cout << "\nStart looping... ...\n";

        pw_main_loop_run(context.getMainLoop());
        
        // backend.setVolume(84, 0.2f);
#endif
        AudioConducker::PipeWireContext context;
        AudioConducker::PipeWireBackend backend(context);
        
        Logger::info("[TEST]Initializing backend...");
        
        backend.initialize();

        auto streams = backend.getStreams();
        spdlog::info("[TEST] Discovered {} streams", streams.size());
        
        if (streams.empty()) {
            Logger::error("Integration test failed: no streams found");
            return 1;
        }

        Logger::info("[TEST] for-loop:");
        for(const auto& stream : streams){
            spdlog::info(
                "Stream: id={} application={} name={} active={} volume={}",
                stream.id,
                stream.application,
                stream.name,
                (stream.isActive ? "yes" : "no"),
                stream.volume
            );
        }

        std::thread loop([&](){ pw_main_loop_run(context.getMainLoop()); });

        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        pw_main_loop_quit(context.getMainLoop());
        
        loop.join();

        Logger::info("[PASS] PipeWire backend integration test");

    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }


    return 0;
}