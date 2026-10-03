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
#include <string>
#include <memory>
#include <thread>
#include <mutex>

/*
  Threading model: nodes_, node_data_, monitors_ are owned by the PipeWire main-loop thread. 
  Cross-thread requests must go through pw_loop_invoke. streams_ is the only shared snapshot (streamMutex_).
*/

namespace AudioConducker{

class PipeWireBackend: public IAudioBackend{
public:
	explicit PipeWireBackend(PipeWireContext& context);

	~PipeWireBackend();

	void initialize();

	void shutdown();

	std::vector<AudioStream> getStreams() override;

	void setVolume(StreamId id, float volume)  override;

	void setVolumeInternal(StreamId id, float volume);

private:
	struct SetVolumeData {
		PipeWireBackend* self;
		StreamId id;
		float volume;
	};

	static int do_set_volume(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data);

	struct NodeData {
		PipeWireBackend* backend;
		StreamId id;
		pw_node* node;
		spa_hook node_listener;

		std::string nodeName;
		std::string application;
		std::string mediaClass;
		std::string mediaName;
	};

	struct QueryVolumeData {
        PipeWireBackend* self;
        StreamId id;
    };

    static int do_shutdown(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data);

	void queryVolume(StreamId id);
	
	static int do_query_volume(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data);
	
	static void onNodeParam(void *data, int seq, uint32_t id, uint32_t index, uint32_t next, const struct spa_pod *param);

	static void onNodeInfo(void *data, const struct pw_node_info *info);
	
	void handleNodeProps(StreamId streamId, uint32_t id, const spa_pod* param);

	void onNodeAdded(StreamId id);

	void onNodeRemoved(StreamId id);

	void onActivityChanged(StreamId id, bool active);

private:
	PipeWireContext& context_;
	
	std::unordered_map<StreamId, struct pw_node*> nodes_;	// use STL to replace struct NodeInfo
	std::unordered_map<StreamId, std::unique_ptr<PipeWireStream>> monitors_;

	std::unordered_map<StreamId, std::unique_ptr<NodeData>> node_data_;
	std::unordered_map<StreamId, AudioStream> streams_;

	std::unique_ptr<NodeObserver> observer_;

	std::mutex streamMutex_;
};

} // namespace AudioConducker
