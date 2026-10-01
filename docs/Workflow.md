# AudioConducker Work Flow


## AudioBackend, the interface

* abstract class, connect to backends from different platform

## PipeWire

Before learning AudioConducker, let's understand PipeWire workflow first:

### Basic Concepts

* PipeWire Daemon(or Server)
    * CORE: managing connections, creating objects, synchronizing, notifying errors...
    * Registry: Daemon's global bulletin board
* Client: Firefox, Spotify, mpv...
* Proxy: interacting with according object. like Registry Proxy, Node Proxy, Client Proxy, Core Proxy...
* Node: global object, created by Client and registried in Registry by CORE
    * Port: input port and output port. A Node with only output ports is often called a source, and a sink is a Node that only possesses input ports.
    * Link: linking two Ports
* Context: local runtime enviornment, is the starting point for establishing connectoins, managing resources, and loading modules

When a Client connects to PipeWire daemon, it will get a `Core Proxy`. This proxy can interact with PipeWire CORE(only one CORE in per PipeWire Daemon, id=0) with calling core methods(like `create_object`), and then get `Rigistry Proxy`. Through Rigistry, Client can observe and bind other objects, then get according Proxy.

If a Clinetg wanna play audio, it will proactively request to create a Node. A Client can request to create multiple Nodes (like Firefox creates nodes for multiple tag pages). When a client requests to create nodes, CORE in daemon will create these objects and export them into Rigistry. At the sametime, broadcast the event (asynchronous event, like `pw_rigistry_event_global`, `pw_node_event_info`) to Clients and notify all listeners.

Every Node has Ports. A Node establishes Link between its Ports and another Node's Ports, forming a data flow. This Link is automatically created by Session Manager(like WirePlumber). 
Other Clients can listen Rigistry to perceive global changes. They can also bind to concrete Nodes to listen their status.

### API Workflow

* `pw_init`: initialize library
* `pw_main_loop_new`: create the main loop
* `pw_context_new`: create context
* `pw_context_connect`: connect to PipeWire Daemon
* `pw_core_get_registry`: get Rigistry Proxy
* `pw_main_loop_run`: start the main loop
* main loop:
    1. Daemon events arrived
    2. Listener callback triggered
    3. Handling events/Updating status
* (receive quit signal)`pw_main_loop_quit`: quit main loop
* `pw_proxy_destroy`: destroy proxies
* `pw_core_disconnect`: disconnect with CORE
* `pw_context_destroy`: destroy context
* `pw_main_loop_destroy`: destroy main loop
* `pw_deinit`: clear library

### PipeWire in AudioConducker

We have 4 classes in platform/pipewire:

- PipeWireContext
- NodeObserver
- PipeWireStream
- PipeWireBackend: public AudioBackend

#### PipeWireContext

> own `pw_main_loop`, `pw_context`, `pw_core`.

Responsible for initializing main loop, context and core; managing their lifecycles.  

Exposing main loop and core.  

Two synchorounous methods:
- roundtrip: a thread-blocking sync
- sync: a cross-thread sync, use `pw_invoke_loop`


#### NodeObserver

> own `pw_registry` and its listener `spa_hook`

Listening Rigistry global/global remove; filtering every stream except of `Stream/Output/Audio` and streams with app name; use callback function (`NodeCallback`, `NodeRemovedCallback`) to notify backend


#### PipeWireStream

> own `pw_stream`

Create capture stream for one node, process audio buffers, call `ActivityDetector` to judge wheather the node is active, callback active status

#### PipeWireBackend

> own `nodes_`, `monitors_`, `node_data_`, `streams_`, `observer_`

Control node's addtion and deletion (`onNodeAdded()`, `onNodeRemoved()`); query or set volume; manage `nodes_`, `monitors_`, `node_data_`, `streams_`, `observer_` map


