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

	static int do_set_volume(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data);

	struct DestroyProxyData {
    	struct pw_proxy* proxy;
	};
	
	static int do_destroy_proxy(struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size, void *user_data);
	
	// void updateVolumeFromProps(const spa_pod* param);

    // static void on_param(void *data, int seq, int32_t id, uint32_t index, uint32_t next, const struct spa_pod *param);

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
	
	// struct NodeInfo{
	// 		AudioStream stream;
	// 		struct pw_node* node;
	// };
	std::unordered_map<StreamId, struct pw_node*> nodes_;	// use STL to replace NodeInfo above
	std::unordered_map<StreamId, std::unique_ptr<PipeWireStream>> monitors_;
    std::unordered_map<StreamId, float> volumes_;
	std::unordered_map<StreamId, std::unique_ptr<NodeData>> node_data_;
	std::unordered_map<StreamId, AudioStream> streams_;

	std::unique_ptr<NodeObserver> observer_;
};

} // namespace AudioConducker
