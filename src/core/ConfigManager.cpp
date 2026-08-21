#include "AudioConducker/core/ConfigManager.h"

namespace AudioConducker{

std::string ConfigManager::getFocusApplication() const{
    return focusApplication_;
}

float ConfigManager::getDuckLevel() const{
    return duckLevel_;
}

} // namespace AudioConducker