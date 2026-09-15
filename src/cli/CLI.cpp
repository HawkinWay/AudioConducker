#include "AudioConducker/cli/CLI.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <string>
#include <stdexcept>

namespace AudioConducker{

CLI::CLI(int argc, char* argv[], AudioBackend& backend): argc_(argc), argv_(argv), monitor_(backend){ }

bool CLI::parse(ConfigManager& config){

    for(int i = 1; i < argc_; i++){
        std::string arg = argv_[i];

        if(arg == "--help" || arg == "-h"){
            printHelp();
            return false;
        }
        else if(arg == "--version" || arg == "-v"){
            printVersion();
            return false;
        }
        else if(arg == "--nodes" || arg == "-n"){
            printNodes();
            // return false;
        }
        else if(arg == "--focus" || arg == "-f"){
            if(i + 1 >= argc_){
                throw std::runtime_error("--focus <app> requires an application name");
            }
            
            config.setFocusApplication(argv_[++i]);
        }
        else if(arg == "--duck" || arg == "-d"){
            if(i + 1 >= argc_){
                throw std::runtime_error("--duck requires a percentage");
            }
            
            const int percentage = std::stoi(argv_[++i]);   // soit: string to integer
            
            if(percentage < 0 || percentage > 100){
                throw std::runtime_error("--duck <percent> must be between 0 and 100");
            }
            
            float amount = static_cast<float>(percentage) / 100.f;
            
            config.setDuckAmount(amount);
        }
        else if(arg == "--log" || arg == "-l"){
            if(i + 1 >= argc_){
                throw std::runtime_error("--log requires a log level: trace/debug/info/warn/error");
            }

            static const std::unordered_set<std::string> logSet = {"trace", "debug", "info", "warn", "error"};  // use static to initialize once

            std::string logLevel = argv_[++i];
            if(logSet.find(logLevel) != logSet.end()){
                config.setLogLevel(logLevel);
            }else{
                throw std::runtime_error("--log <level> must be: trace/debug/info/warn/error");
            }
            
        }
        else{
            throw std::runtime_error("Unknown argument: " + arg + " \nUse -h or --help to find help");
        }
    }

    return true;
}

void CLI::printHelp(){
    std::cout
        << "AudioConducker - Dynamic audio ducking\n\n"
        << "Usage:\n"
        << "  audioconducker --focus <app> [--duck <percent>]\n\n"
        << "Options:\n"
        << "  -f, --focus <app>       Focus application\n"
        << "  -d, --duck <percent>    Duck amount (0-100, default: 80)\n"
        << "  -l, --log <level>       Set log level: trace/debug/info/warn/error\n"
        << "  -n, --nodes             Show all valid nodes\n"
        << "  -h, --help              Show this help message\n"
        << "  -v, --version           Show version\n\n"
        << "Example:\n"
        << "  audioconducker --focus Firefox --duck 80\n";
}

void CLI::printVersion(){
    std::cout << "AudioConducker v0.1.0\n";
}

void CLI::printNodes(){
    std::cout << std::string(55, '=') << " Nodes " << std::string(55, '=') << '\n';

    std::cout 
            << std::left 
            << std::setw(8)  << "ID" 
            << std::setw(30) << "Name" 
            << std::setw(30) << "Application" 
            << std::setw(30) << "Media Class" 
            << std::setw(20) << "Media Name" << '\n';
    std::cout << std::string(116, '-') << '\n';
    
    
    monitor_.update();
    
    std::vector<AudioStream> streams = monitor_.getActiveStreams();
    for(const auto& stream : streams){
        std::cout 
                << std::left 
                << std::setw(8) << stream.id 
                << std::setw(30) << stream.name 
                << std::setw(30) << stream.application 
                << std::setw(30) << stream.mediaClass 
                << std::setw(20) << stream.mediaName << '\n';
    }

    std::cout << std::string(116, '=') << '\n';
}

} // namespace AudioConducker