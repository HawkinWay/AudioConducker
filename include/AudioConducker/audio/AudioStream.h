#pragma once

#include <cstdint>
#include <string>

namespace AudioConducker{

using StreamId = std::uint32_t;

struct AudioStream{
    StreamId id;
    std::string name;
    std::string application;
    std::string mediaClass;
    // std::string mediaName;
    float volume= 1.f;
    bool isActive = false;
    bool controllable = true;
};


} // namespace AudioConducker
