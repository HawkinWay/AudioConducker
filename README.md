# AudioConducker 🎚️🦆

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A cross-platform (**now Linux only**) automatic audio ducking for desktop applications.

AudioConducker automatically lowers the volume of one application when another application starts playing audio.


![gif demo](https://github.com/HawkinWay/AudioConducker/releases/download/untagged-9fbc29427dcee9f764d5/AudioConducker.gif)

---

## Features

🎚️ **Application-based audio ducking**
- Select a focus application
- Automatically reduce the volume of other audio streams while the focus stream is active.

🔄 **Automatic volume restoration**
- Preserve each stream's original volume.
- Restore the original volume when the focus stream becomes inactive or disappears.

📉 **Smooth volume transitions**
- Configurable `attack`, `release` and `hold` times.
- Avoid abrupt volume changes during ducking and restoration.

🔍 **Audio activity detection**
- Monitor audio buffers and use **RMS** determine whether a stream is actively producing audio


🖥️ **Command-line interface**
- Configure focus application, duck amount, logging level, and node inspection from the CLI

🧪 **Automated testing**
- Unit tests with GoogleTest
- Separate PipeWire integration tests for Linux enviornments

---

## Prject Structure

```txt
AudioConducker/
├── CMakeLists.txt              # Build configuration
├── LICENSE                     # MIT License
├── README.md                   # This file
├── .gitignore
├── cmake/
│   └── Dependencies.cmake      # Third-party dependency management (spdlog)
├── docs/
│   ├── architecture.md         # Architecture documentation
│   ├── Workflow.md             # PipeWire workflow reference
│   ├── Command_Line.md         # CLI usage documentation
│   ├── DebugErrors.md          # Debug error log
│   └── AudioConduckerLog.txt   # Application log
├── include/
│   └── AudioConducker/
│       ├── audio/
│       │   ├── ActivityDetector.h
│       │   ├── AudioStream.h
│       │   └── IAudioBackend.h
│       ├── cli/
│       │   └── CLI.h
│       ├── core/
│       │   ├── AudioMonitor.h
│       │   ├── ConfigManager.h
│       │   ├── DuckingEngine.h
│       │   ├── Logger.h
│       │   └── VolumeSmoother.h
│       └── platform/
│           └── pipewire/
│               ├── NodeObserver.h
│               ├── PipeWireBackend.h
│               ├── PipeWireContext.h
│               └── PipeWireStream.h
├── src/
│   ├── main.cpp
│   ├── audio/
│   │   └── ActivityDetector.cpp
│   ├── cli/
│   │   └── CLI.cpp
│   ├── core/
│   │   ├── AudioMonitor.cpp
│   │   ├── ConfigManager.cpp
│   │   ├── DuckingEngine.cpp
│   │   ├── Logger.cpp
│   │   └── VolumeSmoother.cpp
│   └── platform/
│       └── pipewire/
│           ├── NodeObserver.cpp
│           ├── PipeWireBackend.cpp
│           ├── PipeWireContext.cpp
│           └── PipeWireStream.cpp
└── tests/
    ├── CMakeLists.txt
    ├── ActivityDetectorTests.cpp
    ├── ConfigManagerTests.cpp
    ├── DuckingEngineTests.cpp
    ├── PipeWireBackendTests.cpp
    └── VolumeSmootherTests.cpp
```

---

## How It Works

AudioConducker follows a layered architecture with strict platform independence:

```txt
Application (main) → Core Logic → Abstraction Interface → Platform Backend → OS Audio System
```

### Layer Overview

| Layer | Responsibility |
|-------|---------------|
| **Application Layer** | Entry point, configuration initialization, lifecycle management, logging |
| **Core Logic Layer** | Platform-independent business logic: AudioMonitor, ConfigManager, DuckingEngine, VolumeSmoother, Logger |
| **Abstraction Layer** | Unified interface (IAudioBackend) for platform-agnostic audio control |
| **Platform Layer** | Platform-specific implementations (PipeWire, WASAPI, CoreAudio) |
| **System Audio Layer** | OS-level audio services and individual application audio streams |


#### Learn more about:

- [The project architecute](docs/architecture.md)  
- [PipeWire in this project](docs/PipeWireWorkflow.md)

---

## Build Requirements

- **CMake:** >= 3.20
- **C++ Standard:** C++20
- **Linux:** PipeWire development libraries (libpipewire-0.3)
- **Logging:** spdlog (v1.15.3, fetched automatically via CMake FetchContent)

### Dependencies For Linux

Ensure that you have `cmake` and `PipeWire development library` first.

```bash
# Ubuntu/Debian
sudo apt update && sudo apt install -y cmake libpipewire-0.3-dev

# Fedora/RHEL
sudo dnf update && sudo dnf install -y cmake pipewire-devel
```

---

## Quick Start

Clone this repository:

```bash
git clone https://github.com/HawkinWay/AudioConducker.git
cd AudioConducker
```

Configure and build it:

```bash
cmake -B build/ -S . -DCMAKE_BULD_TYPE=Release
# build test options: -DBUILD_TESTS=ON -DBUILD_PIPEWIRE_TESTS=ON
cd build/
cmake --build . --parallel
```

Run it:

```bash
./AudioConducker [OPTIONS]
```

Get [instructions](docs/Command_Line.md) for use:

```bash
./AudioConducker --help
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help message |
| `-f, --focus <app>` | Select the focus (foreground) application |
| `-d, --duck <amount>` | Set duck amount (e.g., `-d 60` reduces background to ~40% volume) |
| `-a, --attack <ms>` | Set attack time in milliseconds |
| `-r, --release <ms>` | Set release time in milliseconds |
| `-h, --hold <ms>` | Set hold time in milliseconds |
| `-l, --log <level>` | Set logging level: trace, debug, info, warn (default), error |
| `-n, --nodes` | Show available audio nodes |
| `-v, --version` | Show version information |
| `-w, --watch-nodes` | Show dynamic node addition/removal events |

---

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

---

*Project name: Audio + Conductor + Ducking. It conducts your desktop's audio
levels so the important stream always gets the stage : )*
