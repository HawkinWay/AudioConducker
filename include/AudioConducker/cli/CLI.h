#pragma once

#include "AudioConducker/core/ConfigManager.h"
#include "AudioConducker/core/AudioMonitor.h"

#include <string>

namespace AudioConducker{

class CLI{
public:
    CLI(int argc, char* argv[]/*, IAudioBackend& backend*/);

    bool parse(ConfigManager& config);

    static void printHelp();
    static void printVersion();
    // void printNodes();
    bool isShowNodes() const;
    bool isWatchingNodes() const;

private:
    int argc_;
    char** argv_;

    bool isShowNodes_{false};
    bool isWatchNodes_{false};

    // AudioMonitor monitor_;
};

} // namespace AudioConducker
