#pragma once

#include "AudioConducker/audio/IAudioBackend.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include "AudioConducker/platform/pipewire/NodeObserver.h"

#include <pipewire/pipewire.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

namespace AudioConducker{

class PipeWireBackend: public AudioBackend{
public:
	explicit PipeWireBackend(PipeWireContext& context);

	~PipeWireBackend();

	std::vector<AudioStream> getStreams() override;

	void setVolume(StreamId id, float volume) override;

private:
	void onNodeAdded(StreamId id);

	PipeWireContext& context_;
	std::unique_ptr<NodeObserver> observer_;
	
	// struct NodeInfo{
	// 	AudioStream stream;
	// 	struct pw_node* node;
	// };

	std::unordered_map<StreamId, struct pw_node*> nodes_;	// use STL to replace NodeInfo above
};

} // namespace AudioConducker
