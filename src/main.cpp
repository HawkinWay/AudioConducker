#include "AudioConducker/cli/CLI.h"
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

int main(int argc, char* argv[])
{
    try{
        CLI cli(argc, argv);

        ConfigManager config;

        if(!cli.parse(config)){
            return 0;
        }

        PipeWireContext context;
        PipeWireBackend backend(context);

        AudioMonitor monitor(backend);

        DuckingEngine engine(backend, config.getDuckRatio());

        backend.initialize();
        std::thread loop(
            [&](){
                pw_main_loop_run(context.getMainLoop());
            }
        );

        while(true){
            monitor.update();

            auto focus = monitor.findStreamByApplication(config.getFocusApplication());
            
            engine.process(focus);

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}