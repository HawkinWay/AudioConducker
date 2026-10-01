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
    duckAmount_ = amount;
}

void ConfigManager::setAttackTime(float attackTime){
    attackTime_ = attackTime;
}
void ConfigManager::setReleaseTime(float releaseTime){
    releaseTime_ = releaseTime;
}
void ConfigManager::setHoldTime(float holdTime){
    holdTime_ = holdTime;
}

void ConfigManager::setLogLevel(const std::string& logLevel){
    logLevel_ = logLevel;
}


} // namespace AudioConducker