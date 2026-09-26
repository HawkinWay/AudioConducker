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
#include <atomic>
#include <csignal>

using namespace AudioConducker;

std::atomic<bool> running{true};

void signalHandler(int signal){
    // spdlog::info("Signal received: {}", signal);
    running.store(false);
}


int main(int argc, char* argv[])
{
    try{

        ConfigManager config;
        
        CLI cli(argc, argv);
        if(!cli.parse(config)){
            return 0;
        }
        spdlog::set_level(spdlog::level::from_str(config.getLogLevel()));
        const auto isShowNodes = cli.isShowNodes();
        const auto isWatchNodes = cli.isWatchingNodes();
        
        PipeWireContext context;
        PipeWireBackend backend(context);
        
        backend.initialize();
        
        AudioMonitor monitor(backend);
        
        std::thread loop(
            [&](){
                pw_main_loop_run(context.getMainLoop());
            }
        );
        
        context.sync();
        // std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        
        
        DuckingEngine engine(backend, config.getDuckAmount());
        
        std::cout << "\n[AudioConducker initialized]\n";

        std::signal(SIGINT, signalHandler);

        if(isShowNodes){
            monitor.update();
            monitor.showNodes();
        }

        std::cout 
                << "\nAudioConducker is running...\n" 
                << "\n*Focus application: " << config.getFocusApplication()
                << "\n*Duck amount: " << config.getDuckAmount() * 100.f << "%\n"
                << "\nPress Ctrl+C to stop.\n" << '\n';

        while(running.load()){
            monitor.update();
            
            if(isWatchNodes)
                monitor.watchNodes();

            auto focus = monitor.findStreamByApplication(config.getFocusApplication());
            
            engine.process(focus);

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        engine.shutDown();
        
        context.sync();
        
        backend.shutdown();
        
        context.quit();
        
        loop.join();

        std::cout << "\nprogram stopped." << '\n';
    }
    catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}