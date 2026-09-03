#pragma once

#include <string>

namespace AudioConducker{

class ConfigManager{
public:
    std::string getFocusApplication() const;
    float getDuckRatio() const;

    void setFocusApplication(const std::string& application);
    void setDuckRatio(float level);
private:
    std::string focusApplication_ = "Firefox";

    float duckRatio_ = 0.2f;
};

} // namespace AudioConducker