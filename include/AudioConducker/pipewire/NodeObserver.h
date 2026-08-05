#pragma once

#include "PipeWireContext.h"
#include <pipewire/pipewire.h>

namespace AudioConducker{

class NodeObserver{
public:
	explicit NodeObserver(PipeWireContext& context);
	
	~NodeObserver();

private:
	static void registry_event_global(
		void *data, 
		uint32_t id,
        uint32_t permissions, 
		const char *type, 
		uint32_t version,
        const struct spa_dict *props
	);

	static const struct pw_registry_events registry_events_;	// contains the events we want to listen to.

	struct pw_registry* registry_;
	struct spa_hook listener_;		// keep track of the listener
};

} // namespace AudioConducker
