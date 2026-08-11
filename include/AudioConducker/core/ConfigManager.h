#pragma once

#include <string>

namespace AudioConducker{

class ConfigManager{
public:
    std::string getFocusApplication() const;

    float getDuckVolume() const;

private:
    std::string focusApplication_ = "Browser";

    float duckLevel_ = 0.2f;
};

} // namespace AudioConducker