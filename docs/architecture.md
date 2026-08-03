# AudioConducker Architecture

This document describes the high-level architecture, component responsibilities, and data flow of **AudioConducker**.

The main design goal is to provide a **cross-platform audio ducking engine** where platform-specific audio APIs are isolated behind a unified abstraction layer.

## 1. Application Layer

*Serves as the application entry point and orchestrator.*

Application/Main

* **Primary File:** `main.cpp`
* **Responsibilities:**
  * Initializes configuration, monitoring, and backend engines.
  * Handles the application lifecycle and handles logs, status updates, and events.

## 2. Core Logic Layer

*Platform-independent business logic for monitoring audio streams and making ducking decisions.*

* **`AudioMonitor`:** Continually tracks system audio streams (active applications, playback state, current volumes).
* **`ConfigManager`:** Loads and manages user configurations (focus application rules, target ducking volumes, fade rates, etc.).
* **`DuckingEngine`:** Evaluates state from `AudioMonitor` against rules in `ConfigManager`.
    * Detects the primary focus stream (e.g., active voice call or primary app).
    * Calculates ducking decisions for secondary streams.
    * Manages smooth volume transitions and dispatches commands to the **Abstraction Layer**.

## 3. Abstraction Layer

*Abstract interface providing platform-agnostic audio control.*

### Data Structures: *AudioStream*
```cpp
struct AudioStream {
    StreamID id;
    std::string name;
    float volume;
    bool isActive;
    // ...
};
```

### AudioBackend(Interface)

*Defines abstract virtual functions for querying streams and adjusting audio levels.*

- `std::vector<AudioStream> getStreams()` Returns all current audio streams.
- `void setVolume(StreamID id, float volume)` Adjusts the volume of a specific audio stream.


## 4. Platform Layer

*Platform-specific implementations of the `AudioBackend` interface.*

- Pipewrie

- WASAPI

- CoreAudio

## 5. System Audio Layer

*The actual OS-level audio services and individual applications generating sound (e.g., Spotify, Chrome, YouTube, Zoom).*

Platform backends communicate with the operating system audio framework, which manages application audio streams.

## Design Principles & Rules

1. **Strict Platform Independence**: The Core Logic Layer (`DuckingEngine`, `AudioMonitor`, `ConfigManager`) never interacts directly with OS-specific audio APIs.

2. **Dependency Inversion**: Platform implementations depend on the abstract AudioBackend interface, allowing new platform backends to be added without modifying the core ducking logic.

3. **Unidirectional Control Flow**: Commands strictly flow downward from `Application` $\rightarrow$ `Core Logic` $\rightarrow$ `Abstraction Interface` $\rightarrow$ `Platform Backend` $\rightarrow$ `OS System Audio`. Data (streams/states) flows upward via the abstraction layer interfaces.

