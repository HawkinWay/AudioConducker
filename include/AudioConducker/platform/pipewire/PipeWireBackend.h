#pragma once

#include "AudioConducker/core/Logger.h"
#include "AudioConducker/audio/IAudioBackend.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include "AudioConducker/platform/pipewire/NodeObserver.h"
#include "AudioConducker/platform/pipewire/PipeWireStream.h"

#include <pipewire/pipewire.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>

namespace AudioConducker{

class PipeWireBackend: public AudioBackend{
public:
	explicit PipeWireBackend(PipeWireContext& context);

	~PipeWireBackend();

	void initialize();

	std::vector<AudioStream> getStreams() override;

	void setVolume(StreamId id, float volume)  override;

	void setVolumeInternal(StreamId id, float volume);

private:
	struct SetVolumeData {
		PipeWireBackend* self;
		StreamId id;
		float volume;
	};

	static int do_set_volume(struct pw_loop* loop, struct spa_source* source, void* data, size_t size, void* user_data);

	struct DestroyProxyData {
    	struct pw_proxy* proxy;
	};

	static int do_destroy_proxy(struct pw_loop* loop, struct spa_source* source, void* data, size_t size, void* user_data);

	void onNodeAdded(StreamId id);

	void onActivityChanged(StreamId id, bool active);

	PipeWireContext& context_;
	
	// struct NodeInfo{
	// 		AudioStream stream;
	// 		struct pw_node* node;
	// };
		
	std::unordered_map<StreamId, struct pw_node*> nodes_;	// use STL to replace NodeInfo above
	std::unordered_map<StreamId, std::unique_ptr<PipeWireStream>> monitors_;
	
	std::unique_ptr<NodeObserver> observer_;
};

} // namespace AudioConducker
