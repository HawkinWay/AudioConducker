#pragma once

#include <cstdint>
#include <string>

namespace AudioConducker{

using StreamId = std::uint32_t;

struct AudioStream{
    StreamId id;
    std::string name;
    std::string application;
    float volume= 1.f;
    bool isActive = false;
};


} // namespace AudioConducker