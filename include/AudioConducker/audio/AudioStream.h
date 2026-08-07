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
    float volume= 1.f;
    bool isActive = false;
    bool controllable = false;
};


} // namespace AudioConducker