#pragma once

#include "AudioConducker/core/ConfigManager.h"
#include "AudioConducker/core/AudioMonitor.h"

#include <string>

namespace AudioConducker{

class CLI{
public:
    CLI(int argc, char* argv[], AudioBackend& backend);

    bool parse(ConfigManager& config);

    static void printHelp();
    static void printVersion();
    void printNodes();

private:
    int argc_;
    char** argv_;

    AudioMonitor monitor_;
};

} // namespace AudioConducker
