#include "AudioConducker/cli/CLI.h"

#include <iostream>
#include <stdexcept>

namespace AudioConducker{

CLI::CLI(int argc, char* argv[]): argc_(argc), argv_(argv){ }

bool CLI::parse(ConfigManager& config) const{

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
        else if(arg == "--focus" || arg == "-f"){
            if(i + 1 > argc_){
                throw std::runtime_error("--focus requires an application name");
            }

            config.setFocusApplication(argv_[++i]);
        }
        else if(arg == "--duck" || arg == "-d"){
            if(i + 1 >= argc_){
                throw std::runtime_error("--duck requires a percentage");
            }

            const int percentage = std::stoi(argv_[++i]);

            if(percentage < 0 || percentage > 100){
                throw std::runtime_error("--duck must be between 0 and 100");
            }

            float ratio = 1.f - static_cast<float>(percentage) / 100.f;

            config.setDuckRatio(ratio);
        }
        else{
            throw std::runtime_error("Unknown argument: " + arg + " Use -h or --help to find help");
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
        << "  -h, --help              Show this help message\n"
        << "  -v, --version           Show version\n\n"
        << "Example:\n"
        << "  audioconducker --focus Firefox --duck 80\n";
}

void CLI::printVersion(){
    std::cout << "AudioConducker v0.1.0\n";
}

} // namespace AudioConducker