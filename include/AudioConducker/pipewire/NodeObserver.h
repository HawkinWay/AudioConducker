#pragma once

#include <pipewire/pipewire.h>

namespace AudioConducker{

class NodeObserver{
public:
	NodeObserver();
	~NodeObserver();

	void run();
private:
	struct Impl;
	Impl* impl_;
};

} // namespace AudioConducker
