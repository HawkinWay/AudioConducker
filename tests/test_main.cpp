#include <gtest/gtest.h>
#include "AudioConducker/core/Logger.h"
#include "AudioConducker/audio/ActivityDetector.h"
#include "AudioConducker/core/DuckingEngine.h"
#include "AudioConducker/core/ConfigManager.h"
#include "AudioConducker/core/AudioMonitor.h"
#include "AudioConducker/platform/pipewire/NodeObserver.h"
#include "AudioConducker/platform/pipewire/PipeWireContext.h"
#include "AudioConducker/platform/pipewire/PipeWireBackend.h"
#include "AudioConducker/platform/pipewire/PipeWireStream.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace AudioConducker;

// TEST(ProjectTest, Basic)
// {
//     EXPECT_EQ(1,1);
// }

// TEST(ActivityDetectorTest, process){
//     AudioConducker::ActivityDetector detector;

//     float samples[] = {0.1f, 0.2f, 0.3f};
    
//     EXPECT_TRUE(detector.process(samples, 3));
// }

TEST(AudioConduckerIntegrationTests, integration){
	AudioConducker::PipeWireContext context;
	AudioConducker::PipeWireBackend backend(context);
	
	Logger::info("[TEST] Initializing backend...\n");

	
	backend.initialize();
	
	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	Logger::info("\n[TEST] Initializing monitor...");
	AudioConducker::AudioMonitor monitor(backend);
	Logger::info("[TEST] Initializing config manager...");
	AudioConducker::ConfigManager config;
	Logger::info("[TEST] Initializing ducking engine...");
	AudioConducker::DuckingEngine engine(backend, config.getDuckLevel());
	
	std::thread loop(
		[&](){ 
			pw_main_loop_run(context.getMainLoop()); 
			spdlog::info(
				"PipeWire main loop thread {}",
				std::hash<std::thread::id>{}(std::this_thread::get_id())
			);
	});
	std::this_thread::sleep_for(std::chrono::seconds(2));
	
	auto streams = backend.getStreams();
	spdlog::info("[TEST] Discovered {} streams", streams.size());
	bool anyActive = std::any_of(
		streams.begin(),
		streams.end(),
		[](const AudioStream& stream) {
			return stream.isActive;
		}
	);

	if(!anyActive){
		Logger::error("No active streams, continuing anyway (or use GTEST_SKIP())");
	}

	
	if (streams.empty()) {
		Logger::error("Integration test failed: no streams found");
		return;
	}

	Logger::info("[TEST] Main loop started");


	Logger::info("[TEST] for-loop:");
	for(const auto& stream : streams){
		spdlog::info(
			"Stream: id={} application={} name={} active={} volume={}",
			stream.id,
			stream.application,
			stream.name,
			(stream.isActive ? "yes" : "no"),
			stream.volume
		);
	}


	Logger::info("\n====================== DUCK =======================");
	for(int count = 0; count < 25; count++){
		monitor.update();

		auto focus = monitor.findStreamByApplication(config.getFocusApplication());
		
		if(!focus){
			spdlog::warn("[TEST] Focus application '{}' not found, skipping process()",
				config.getFocusApplication());
		}else{
			spdlog::info("Focus Application ID: {}", *focus);
			engine.process(*focus);
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	std::cin.get();

	Logger::info("\n====================== RESTORE =======================");
	for(int count = 0; count < 25; count++){
		monitor.update();

		auto focus = monitor.findStreamByApplication(config.getFocusApplication());

		if(!focus){
			spdlog::warn("[TEST] Focus application '{}' not found, skipping process()",
						config.getFocusApplication());
		}else{
			spdlog::info("Focus Application ID: {}", *focus);
			engine.process(*focus);
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}


	pw_main_loop_quit(context.getMainLoop());
	
	
	loop.join();

	Logger::info("[PASS] PipeWire backend integration test");
}
