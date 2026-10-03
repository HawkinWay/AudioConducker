#pragma once

#include <string>
#include <algorithm>

namespace AudioConducker{

class ConfigManager{
public:
    std::string getFocusApplication() const;
    float getDuckAmount() const;
    float getAttackTime() const;
    float getReleaseTime() const;
    float getHoldTime() const;
    std::string getLogLevel() const;
    
    void setFocusApplication(const std::string& application);
    void setDuckAmount(float amount);
    void setAttackTime(float attackTime);
    void setReleaseTime(float releaseTime);
    void setHoldTime(float holdTime);
    void setLogLevel(const std::string& logLevel);
private:
    std::string focusApplication_{"Firefox"};

    float duckAmount_{0.5f};
    float attackTime_{0.08f};
    float releaseTime_{0.5f};
    float holdTime_{0.4f};

    std::string logLevel_{"warn"};
};

} // namespace AudioConducker