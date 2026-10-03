#include "AudioConducker/core/VolumeSmoother.h"

namespace AudioConducker{

VolumeSmoother::VolumeSmoother(float attackTime, float releaseTime): attackTime_(attackTime), releaseTime_(releaseTime){
    
}

void VolumeSmoother::setCurrent(float current){
    current_ = std::clamp(current, 0.f, 1.f);
}


void VolumeSmoother::setTarget(float target){
    target_ = std::clamp(target, 0.f, 1.f);
}

float VolumeSmoother::getCurrent() const{
    return current_;
}

float VolumeSmoother::getTarget() const{
    return target_;
}

float VolumeSmoother::process(float deltaTime){
    assert(deltaTime >= 0.f);

    auto distance = target_ - current_;

    float time = distance < 0.f ? attackTime_ : releaseTime_;

    if(time == 0.f){
        current_ = target_;
        return current_;
    }

    float maxChange = deltaTime / time;

    if(std::abs(distance) <= maxChange){
        current_ = target_;
    }else{
        current_ += std::copysign(maxChange, distance);
    }

    return current_;
}

bool VolumeSmoother::isSmoothing() const{
    return std::abs(target_ - current_) > kEpsilon_;
    //return current_ != target_;
}

} // namespace AudioConducker