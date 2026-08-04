#include "AudioConducker/core/Logger.h"
#include "AudioConducker/pipewire/NodeObserver.h"
#include "AudioConducker/pipewire/PipeWireContext.h"
#include <iostream>

int main()
{
    // test logger
    AudioConducker::Logger::init();

    AudioConducker::Logger::info("Application started");

    AudioConducker::Logger::debug("Debug message");

    AudioConducker::Logger::error("Example error");

    //test pipewire
    AudioConducker::NodeObserver observer;
    observer.run();

    try{
        AudioConducker::PipeWireContext context;
        std::cout << "connected to pipewire\n";
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }

    return 0;
}