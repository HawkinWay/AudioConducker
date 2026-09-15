#pragma once

#include <string>

namespace AudioConducker{

class ConfigManager{
public:
    std::string getFocusApplication() const;
    float getDuckAmount() const;
    std::string getLogLevel() const;

    void setFocusApplication(const std::string& application);
    void setDuckAmount(float amount);
    void setLogLevel(const std::string& logLevel);
private:
    std::string focusApplication_{"Firefox"};

    float duckAmount_{0.2f};

    std::string logLevel_{"warn"};
};

} // namespace AudioConducker