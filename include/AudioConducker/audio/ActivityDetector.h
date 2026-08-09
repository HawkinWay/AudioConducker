#pragma once

#include <cstddef>

namespace AudioConducker{

class ActivityDetector{
public:
    explicit ActivityDetector(float threshold = 0.01f);

    bool process(const float* samples, size_t count);

private:
    float threshold_;
};

} // namespace AudioConducker