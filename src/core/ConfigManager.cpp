#include "AudioConducker/core/ConfigManager.h"

namespace AudioConducker{

std::string ConfigManager::getFocusApplication() const{
    return focusApplication_;
}

float ConfigManager::getDuckAmount() const{
    return duckAmount_;
}

std::string ConfigManager::getLogLevel() const{
    return logLevel_;
}


void ConfigManager::setFocusApplication(const std::string& application){
    focusApplication_ = application;
}

void ConfigManager::setDuckAmount(float amount){
    duckAmount_ = amount;
}

void ConfigManager::setLogLevel(const std::string& logLevel){
    logLevel_ = logLevel;
}


} // namespace AudioConducker