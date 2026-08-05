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

    AudioConducker::Logger::error("Example error\n");

    //test pipewire
    try{
        AudioConducker::PipeWireContext context;
        std::cout << "Pipewire context initialized.\n";

        AudioConducker::NodeObserver observer(context);
        std::cout << "Start looping... ...\n";

	    pw_main_loop_run(context.getMainLoop());
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }

    return 0;
}
