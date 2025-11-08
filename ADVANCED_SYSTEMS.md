# Advanced Systems Integration

This directory contains advanced engine systems for physics, networking, and scripting.

## Systems Overview

### 1. Physics System (`physics_system.py`)
- **Engine**: PyMunk (Chipmunk2D)
- **Features**:
  - 2D rigid body physics
  - Static, dynamic, and kinematic bodies
  - Box, circle, and segment shapes
  - Collision detection and response
  - Raycasting and spatial queries
  - Debug visualization
  - Configurable physics properties (friction, elasticity, mass)

**Example**:
```python
from physics_system import PhysicsSystem

physics = PhysicsSystem(gravity=(0, 980))
box = physics.create_dynamic_body('box', (50, 50), (400, 100), mass=1.0)
physics.update(1/60)  # Update at 60 FPS
```

### 2. Network System (`network_system.py`)
- **Features**:
  - HTTP/REST API client
  - WebSocket client for real-time communication
  - WebSocket server for hosting multiplayer
  - JSON serialization support
  - Async networking with threading
  - File downloads
  - Broadcast and unicast messaging

**Example**:
```python
from network_system import NetworkSystem

network = NetworkSystem()

# HTTP requests
network.set_http_base_url("https://api.example.com")
response = network.http_client.get("/data")

# WebSocket server
server = network.start_websocket_server("localhost", 8765)
server.on_message(lambda client, msg: print(f"Received: {msg}"))

# WebSocket client
client = network.connect_websocket("ws://localhost:8765")
client.send("Hello!")
```

### 3. Scripting System (`scripting_system.py`)
- **Engine**: Lupa (Lua via LuaJIT)
- **Features**:
  - Lua script execution
  - Python-Lua interoperability
  - Function registration (Python ↔ Lua)
  - Module/table registration
  - Script hot-reloading
  - Entity behavior scripting
  - Modding support

**Example**:
```python
from scripting_system import ScriptingSystem

scripting = ScriptingSystem()

# Register Python function for Lua
def spawn_enemy(x, y):
    print(f"Spawning at {x}, {y}")

scripting.register_function("spawn_enemy", spawn_enemy)

# Execute Lua code
scripting.execute_string("""
    function on_update(dt)
        spawn_enemy(100, 200)
    end
""")

scripting.call_global_function("on_update", 0.016)
```

## Demos

### Advanced Demo (`advanced_demo.py`)
Interactive physics simulation with all three systems integrated.

**Controls**:
- `SPACE` - Spawn box
- `C` - Spawn circle
- `R` - Reset simulation
- `Left Click` - Apply force to nearby objects
- `ESC` - Quit

**Run**:
```bash
python advanced_demo.py
```

### Multiplayer Example (`multiplayer_example.py`)
Demonstrates client-server networking with WebSocket.

**Run Server**:
```bash
python multiplayer_example.py server
```

**Run Client** (in separate terminal):
```bash
python multiplayer_example.py
```

## Dependencies

All dependencies are already installed:
- `pymunk` - 2D physics engine
- `lupa` - Lua scripting integration
- `requests` - HTTP client
- `websockets` - WebSocket support

## Integration with Existing Engine

These systems can be integrated with your existing ECS, renderer, and other components:

```python
from physics_system import PhysicsSystem
from network_system import NetworkSystem
from scripting_system import ScriptingSystem

class GameEngine:
    def __init__(self):
        self.physics = PhysicsSystem()
        self.network = NetworkSystem()
        self.scripting = ScriptingSystem()
        
        # Register engine functions for Lua
        self.scripting.register_function("log", self.log)
        
    def update(self, dt):
        self.physics.update(dt)
        self.scripting.call_global_function("on_update", dt)
```

## Use Cases

### Physics
- Platformer games
- Puzzle games with physics
- Particle systems
- Ragdoll animation
- Vehicle simulation

### Networking
- Multiplayer games
- Leaderboards
- Cloud saves
- Content delivery
- Matchmaking

### Scripting
- Modding support
- Game logic separation
- Rapid prototyping
- Designer-friendly scripting
- Live reloading during development

## Performance Notes

- **Physics**: PyMunk is highly optimized C code, suitable for 100s of bodies at 60 FPS
- **Networking**: Async design prevents blocking game loop
- **Scripting**: LuaJIT provides near-native performance for hot code paths

## Next Steps

Consider adding:
1. Physics-ECS integration for component-based physics
2. Network prediction and interpolation for smooth multiplayer
3. Lua bindings for all engine systems
4. Physics material presets
5. Network protocol encryption
6. Script sandboxing for secure modding

Happy game development! 🎮
