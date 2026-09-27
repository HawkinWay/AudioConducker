#pragma once

#include <algorithm>
#include <cmath>
#include <cassert>

namespace AudioConducker{

class VolumeSmoother{
public:
    VolumeSmoother(float attackTime, float releaseTime);

    void setCurrent(float current);
    void setTarget(float target);

    float getCurrent() const;
    float getTarget() const;

    float process(float deltaTime);

    bool isSmoothing() const;

private:
    static constexpr float kEpsilon_{0.0001f};

    float current_{1.f};
    float target_{1.f};

    float attackTime_;
    float releaseTime_;
};

} // namespace AudioConducker