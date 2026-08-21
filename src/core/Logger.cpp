#include "AudioConducker/core/Logger.h"


namespace AudioConducker{

void Logger::init(){
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("AudioConducker logger initialized");
}

void Logger::debug(const std::string& message){
    spdlog::debug(message);
}

void Logger::info(const std::string& message){
    spdlog::info(message);
}

void Logger::error(const std::string& message){
    spdlog::error(message);
}

} // namespace AudioConducker