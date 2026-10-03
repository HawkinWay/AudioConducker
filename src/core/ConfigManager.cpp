#include "AudioConducker/core/ConfigManager.h"

namespace AudioConducker{

std::string ConfigManager::getFocusApplication() const{
    return focusApplication_;
}

float ConfigManager::getDuckAmount() const{
    return duckAmount_;
}

float ConfigManager::getAttackTime() const{
    return attackTime_;
}
float ConfigManager::getReleaseTime() const{
    return releaseTime_;
}
float ConfigManager::getHoldTime() const{
    return holdTime_;
}

std::string ConfigManager::getLogLevel() const{
    return logLevel_;
}


void ConfigManager::setFocusApplication(const std::string& application){
    focusApplication_ = application;
}

void ConfigManager::setDuckAmount(float amount){
    duckAmount_ = std::clamp(amount, 0.f, 100.f);
}

void ConfigManager::setAttackTime(float attackTime){
    attackTime_ = std::clamp(attackTime, 0.005f, 0.5f);
}
void ConfigManager::setReleaseTime(float releaseTime){
    releaseTime_ = std::clamp(releaseTime, 0.005f, 3.f);
}
void ConfigManager::setHoldTime(float holdTime){
    holdTime_ = std::clamp(holdTime, 0.005f, 2.f);
}

void ConfigManager::setLogLevel(const std::string& logLevel){
    logLevel_ = logLevel;
}


} // namespace AudioConducker