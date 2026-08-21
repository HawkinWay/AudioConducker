#pragma once

#include <string>

namespace AudioConducker{

class ConfigManager{
public:
    std::string getFocusApplication() const;

    float getDuckLevel() const;

private:
    std::string focusApplication_ = "mpv";

    float duckLevel_ = 0.2f;
};

} // namespace AudioConducker