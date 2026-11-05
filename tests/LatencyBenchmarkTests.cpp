#include <gtest/gtest.h>

#include "AudioConducker/audio/IAudioBackend.h"
#include "AudioConducker/audio/ActivityDetector.h"
#include "AudioConducker/core/AudioMonitor.h"
#include "AudioConducker/core/ConfigManager.h"
#include "AudioConducker/core/DuckingEngine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <thread>
#include <vector>

using namespace AudioConducker;

// DuckingEngine.h already declares a global `Clock` alias (steady_clock).
using namespace std::chrono_literals;

namespace{

constexpr int kRuns = 100;
constexpr auto kTick = 20ms;
constexpr int kMaxTicksPerRun = 100;
constexpr int kMaxFailedRuns = 5;

constexpr StreamId kFocusId = 1;
constexpr StreamId kBackgroundId = 2;
constexpr float kOriginalVolume = 1.f;
constexpr float kEpsilon = 1e-4f;

class LatencyMockBackend : public IAudioBackend{
public:
    struct AppliedVolume{
        StreamId id;
        float volume;
        Clock::time_point at;        // timestamp of the setVolume() call
    };

    std::vector<AudioStream> streams;
    std::vector<AppliedVolume> appliedHistory;

    std::vector<AudioStream> getStreams() override{
        return streams;
    }

    void setVolume(StreamId id, float volume) override{
        appliedHistory.push_back({id, volume, Clock::now()});
        for(auto& s : streams) if(s.id == id) s.volume = volume;
    }
};

AudioStream makeStream(StreamId id, const std::string& application, bool isActive){
    return AudioStream{
        .id = id,
        .name = "node-" + std::to_string(id),
        .application = application,
        .mediaClass = "Stream/Output/Audio",
        .mediaName = "media-" + std::to_string(id),
        .volume = kOriginalVolume,
        .channelCount = 2,
        .isActive = isActive,
        .controllable = true,
    };
}

} // namespace

TEST(MockControlPathLatencyTest, TriggerToFirstDuckCommand) {
    constexpr int kRuns = 100;
    constexpr auto kTick = std::chrono::milliseconds(20);

    ConfigManager config;
    std::vector<double> latenciesMs;
    latenciesMs.reserve(kRuns);

    for (int run = 0; run < kRuns; ++run) {
        LatencyMockBackend backend;
        backend.streams = {
            makeStream(kFocusId, "FocusApp", false),
            makeStream(kBackgroundId, "BackgroundApp", true),
        };

        AudioMonitor monitor{backend};
        DuckingEngine engine{
            backend,
            config.getDuckAmount(),
            config.getAttackTime(),
            config.getReleaseTime(),
            config.getHoldTime(),
        };

        auto lastTime = Clock::now();

        const auto step = [&] {
            const auto now = Clock::now();
            const float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;

            monitor.update();
            engine.process(monitor.findStreamByApplication("FocusApp"), dt);
        };

        step();
        std::this_thread::sleep_for(kTick);
        step();
        ASSERT_TRUE(backend.appliedHistory.empty()) << "Unexpected volume change before trigger";

        backend.streams[0].isActive = true;
        const auto triggerTime = Clock::now();

        // wait until the first background volume request.
        std::optional<Clock::time_point> commandTime;

        for (int tick = 0; tick < 100 && !commandTime; ++tick) {
            step();

            for (const auto& record : backend.appliedHistory) {
                if (record.id == kBackgroundId && record.at >= triggerTime) {
                    commandTime = record.at;
                    break;
                }
            }

            if (!commandTime) std::this_thread::sleep_for(kTick);
        }

        ASSERT_TRUE(commandTime) << "No Ducking command received in run " << run;

        const double latencyMs = 
            std::chrono::duration<double, std::milli>(*commandTime - triggerTime).count();

        latenciesMs.push_back(latencyMs);
    }

    std::sort(latenciesMs.begin(), latenciesMs.end());

    const double p50 = (latenciesMs[49] + latenciesMs[50]) / 2.0;
    const double p95 = latenciesMs[94];

    std::cout << "Mock trigger-to-command latency (ms): "
              << "\nruns =" << latenciesMs.size()
              << "\nP50 =" << p50
              << "\nP95 =" << p95
              << "\nmax =" << latenciesMs.back()
              << '\n';
}