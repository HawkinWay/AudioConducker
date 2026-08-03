#include "AudioConducker/core/Logger.h"


int main()
{

    AudioConducker::Logger::init();


    AudioConducker::Logger::info("Application started");


    AudioConducker::Logger::debug("Debug message");


    AudioConducker::Logger::error("Example error");


    return 0;
}