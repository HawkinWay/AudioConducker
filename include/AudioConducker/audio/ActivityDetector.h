#pragma once

#include <cstddef>

namespace AudioConducker{

class ActivityDetector{
public:
    explicit ActivityDetector(float threshold = 0.05f);

    bool process(const float* samples, size_t count);

    float getRMS(const float* samples, size_t count) const;

private:
    float threshold_;
};

} // namespace AudioConducker