#pragma once

#include "AudioConducker/core/Logger.h"
#include "AudioConducker/audio/AudioStream.h"
#include "PipeWireContext.h"
#include <pipewire/pipewire.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <utility>

namespace AudioConducker{

class NodeObserver{
public:
	using NodeCallback = std::function<void(StreamId)>;
	using NodeRemovedCallback = std::function<void(StreamId)>;

	NodeObserver(PipeWireContext& context, NodeCallback callback, NodeRemovedCallback removedCallback);
	
	~NodeObserver();

	// const std::vector<AudioStream>& getStreams() const;

	// std::optional<std::reference_wrapper<AudioStream>> getStreamById(StreamId id);

	struct pw_registry* getRegistry() const;

	// void setActive(StreamId id, bool active);

private:
	static void registry_event_global(
		void *data, 
		uint32_t id,
        uint32_t permissions, 
		const char *type, 
		uint32_t version,
        const struct spa_dict *props
	);

	static void registry_event_global_remove(void *data, uint32_t id);

	static const struct pw_registry_events registry_events_;	// contains the events we want to listen to.

	struct pw_registry* registry_;
	struct spa_hook listener_;		// keep track of the listener

	// std::vector<AudioStream> streams_;

	NodeCallback callback_;
	NodeRemovedCallback removedCallback_;
};

} // namespace AudioConducker
