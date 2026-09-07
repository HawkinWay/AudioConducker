#pragma once

#include <string>

namespace AudioConducker{

class ConfigManager{
public:
    std::string getFocusApplication() const;
    float getDuckAmount() const;

    void setFocusApplication(const std::string& application);
    void setDuckAmount(float amount);
private:
    std::string focusApplication_{"Firefox"};

    float duckAmount_{0.2f};
};

} // namespace AudioConducker