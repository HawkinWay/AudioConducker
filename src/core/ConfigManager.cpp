#include "AudioConducker/core/ConfigManager.h"

namespace AudioConducker{

std::string ConfigManager::getFocusApplication() const{
    return focusApplication_;
}

float ConfigManager::getDuckVolume() const{
    return duckVolume_;
}

} // namespace AudioConducker