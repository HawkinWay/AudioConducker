#include "AudioConducker/core/ConfigManager.h"

namespace AudioConducker{

std::string ConfigManager::getFocusApplication() const{
    return focusApplication_;
}

float ConfigManager::getDuckRatio() const{
    return duckRatio_;
}

void ConfigManager::setFocusApplication(const std::string& application){
    focusApplication_ = application;
}

void ConfigManager::setDuckRatio(float level){
    duckRatio_ = level;
}

} // namespace AudioConducker