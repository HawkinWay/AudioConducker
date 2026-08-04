#include "AudioConducker/core/Logger.h"
#include "pipewire/NodeObserver.h"

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

    return 0;
}