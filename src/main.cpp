#include "AudioConducker/core/Logger.h"
#include "AudioConducker/platform/pipewire/NodeObserver.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include "AudioConducker/platform/pipewire/PipeWireBackend.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    // test logger
    AudioConducker::Logger::init();

    AudioConducker::Logger::info("Application started");

    AudioConducker::Logger::debug("Debug message");

    AudioConducker::Logger::error("Example error\n");

    //test pipewire
    try{
        AudioConducker::PipeWireContext context;
        std::cout << "Pipewire context initialized.\n";

        
        AudioConducker::PipeWireBackend backend(context);
        std::cout << "\nStart looping... ...\n";

        pw_main_loop_run(context.getMainLoop());
        
        // backend.setVolume(84, 0.2f);

    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }


    return 0;
}
