#pragma once

#include <pipewire/pipewire.h>

namespace AudioConducker{

class PipeWireContext{
public:
    PipeWireContext();

    ~PipeWireContext();

    pw_core* getCore() const;

private:
    struct pw_main_loop* loop_;
    struct pw_context* context_;
    struct pw_core* core_;
};

} // namespace AudioConducker
