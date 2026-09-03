#include "AudioConducker/audio/ActivityDetector.h"

#include <cmath>

namespace AudioConducker{

ActivityDetector::ActivityDetector(float threshold): threshold_(threshold){

}

bool ActivityDetector::process(const float* samples, size_t count){
    if(count == 0)  return false;

    float sum =  0.f;
    for(size_t i = 0; i < count; i++){
        sum += samples[i] * samples[i];
    }

    float rms = std::sqrt(sum / count);
    return rms > threshold_;
}

float ActivityDetector::getRMS(const float* samples, size_t count) const{
    if(count == 0)  return false;

    float sum =  0.f;
    for(size_t i = 0; i < count; i++){
        sum += samples[i] * samples[i];
    }

    float rms = std::sqrt(sum / count);

    return rms;
}

} // namespace AudioConducker