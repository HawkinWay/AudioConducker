#include "NodeObserver.h"
#include <iostream>

namespace AudioConducker{

NodeObserver::NodeObserver(): impl_(nullptr){
    pw_init(nullptr, nullptr);
    std::cout << "Pipewire initialized.\n"; 
}

NodeObserver::~NodeObserver(){
    pw_deinit();
}

void NodeObserver::run(){
    std::cout << "Running Pipewire observer...\n";
}

} // namespace AudioConducker