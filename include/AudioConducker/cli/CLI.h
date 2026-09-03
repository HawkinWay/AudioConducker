#pragma once

#include "AudioConducker/core/ConfigManager.h"

#include <string>

namespace AudioConducker{

class CLI{
public:
    CLI(int argc, char* argv[]);

    bool parse(ConfigManager& config) const;

    static void printHelp();
    static void printVersion();

private:
    int argc_;
    char** argv_;
};

} // namespace AudioConducker
