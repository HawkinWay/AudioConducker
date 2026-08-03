#pragma once

#include <string>

namespace AudioConducker{

class Logger{
public:
    static void init();

    static void debug(const std::string& message);

    static void info(const std::string& message);

    static void error(const std::string& message);
};

} // namespace AudioConducker