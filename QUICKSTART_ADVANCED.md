# Quick Start Guide - Advanced Systems

## 🚀 Getting Started in 5 Minutes

### 1. Verify Installation
```bash
cd C:\Users\alecc\omega-engine
python -c "from physics_system import PhysicsSystem; from network_system import NetworkSystem; from scripting_system import ScriptingSystem; print('✅ All systems ready!')"
```

### 2. Run Your First Physics Simulation
```python
from physics_system import PhysicsSystem

# Create physics world
physics = PhysicsSystem(gravity=(0, 980))

# Add ground
ground = physics.create_static_body('segment', (800, 0), (400, 550))

# Add falling box
box = physics.create_dynamic_body('box', (50, 50), (400, 100), mass=1.0)

# Simulate 60 frames
for i in range(60):
    physics.update(1/60)
    print(f"Frame {i}: Box at {box.position}")
```

### 3. Create a Simple Multiplayer Server
```python
from network_system import WebSocketServer
import time

server = WebSocketServer("localhost", 8765)

def on_message(client, message):
    print(f"Received: {message}")
    server.broadcast(f"Echo: {message}")

server.on_message(on_message)
server.start()

print("Server running on ws://localhost:8765")
time.sleep(60)  # Run for 1 minute
server.stop()
```

### 4. Execute Lua Script
```python
from scripting_system import ScriptingSystem

scripting = ScriptingSystem()

# Register Python function
def log(msg):
    print(f"[LUA] {msg}")

scripting.register_function("log", log)

# Execute Lua
scripting.execute_string("""
    log("Hello from Lua!")
    
    function calculate_damage(base, crit)
        if crit then
            return base * 2
        end
        return base
    end
""")

# Call Lua function
damage = scripting.call_global_function("calculate_damage", 50, True)
print(f"Damage: {damage}")  # Output: 100
```

## 📖 Common Patterns

### Pattern 1: Physics-Based Game Object
```python
class PhysicsEntity:
    def __init__(self, physics_system, x, y):
        self.body = physics_system.create_dynamic_body(
            'box', (32, 32), (x, y), mass=1.0
        )
    
    def update(self):
        self.x, self.y = self.body.position
    
    def apply_force(self, fx, fy):
        self.body.apply_force((fx, fy))
```

### Pattern 2: Networked Player
```python
class NetworkedPlayer:
    def __init__(self, client, player_id):
        self.client = client
        self.id = player_id
        self.x = 0
        self.y = 0
    
    def send_position(self):
        self.client.send_json({
            "type": "move",
            "id": self.id,
            "x": self.x,
            "y": self.y
        })
```

### Pattern 3: Scripted Behavior
```python
class ScriptedEnemy:
    def __init__(self, scripting, script_name):
        self.scripting = scripting
        self.script = scripting.get_script(script_name)
        self.health = 100
    
    def update(self, dt):
        # Call Lua update
        self.script.call_function("on_update", self, dt)
    
    def take_damage(self, amount):
        self.health -= amount
        self.script.call_function("on_damage", self, amount)
```

## 🎮 Complete Game Loop Example

```python
from physics_system import PhysicsSystem
from network_system import NetworkSystem
from scripting_system import ScriptingSystem
import time

class Game:
    def __init__(self):
        self.physics = PhysicsSystem()
        self.network = NetworkSystem()
        self.scripting = ScriptingSystem()
        self.running = True
        
        # Setup
        self.setup_game()
    
    def setup_game(self):
        # Create physics world
        self.ground = self.physics.create_static_body(
            'segment', (800, 0), (400, 550)
        )
        
        # Load game scripts
        self.scripting.execute_string("""
            function on_game_start()
                print("Game started!")
            end
            
            function on_game_update(dt)
                -- Game logic here
            end
        """)
        
        # Start multiplayer server (optional)
        # self.server = self.network.start_websocket_server("localhost", 8765)
    
    def update(self, dt):
        self.physics.update(dt)
        self.scripting.call_global_function("on_game_update", dt)
    
    def run(self):
        self.scripting.call_global_function("on_game_start")
        
        last_time = time.time()
        while self.running:
            current_time = time.time()
            dt = current_time - last_time
            last_time = current_time
            
            self.update(dt)
            time.sleep(1/60)  # 60 FPS

if __name__ == "__main__":
    game = Game()
    game.run()
```

## 🔥 Advanced Features

### Physics Collision Callbacks
```python
def on_collision(arbiter, space, data):
    print("Collision detected!")
    return True

physics.add_collision_handler(1, 2, begin=on_collision)
```

### Network Message Handling
```python
client = network.connect_websocket("ws://localhost:8765")

def handle_message(msg):
    import json
    data = json.loads(msg)
    if data["type"] == "player_move":
        update_player(data["id"], data["x"], data["y"])

client.on_message(handle_message)
```

### Lua-Python Data Sharing
```python
# Python to Lua
game_state = {"score": 100, "level": 5}
scripting.register_module("GameState", game_state)

# Lua can now access: GameState.score, GameState.level

# Lua to Python
lua_data = scripting.get_script("game").get_table("player_data")
print(lua_data)  # Python dict
```

## 📝 Tips & Best Practices

1. **Physics**: Use substeps for accuracy: `physics.update(dt, substeps=4)`
2. **Networking**: Always handle connection errors gracefully
3. **Scripting**: Separate game logic from engine code for flexibility
4. **Performance**: Profile your physics world if > 200 bodies
5. **Debugging**: Use `physics.enable_debug_draw(screen)` for visualization

## 🐛 Troubleshooting

### Physics objects falling through floor?
- Increase substeps: `physics.update(dt, substeps=5)`
- Check body types (dynamic vs static)
- Verify collision shape sizes

### Network connection failing?
- Check firewall settings
- Verify server is running first
- Use `localhost` for local testing

### Lua function not found?
- Check script loaded: `scripting.get_script("name")`
- Verify function name spelling
- Use `scripting.execute_string()` to test

## 🎯 Next Actions

1. ✅ Copy examples above and modify for your game
2. ✅ Read ADVANCED_SYSTEMS.md for detailed documentation
3. ✅ Run `python advanced_demo.py` to see systems in action
4. ✅ Experiment with physics parameters
5. ✅ Build your first networked game!

---

**You're ready to build amazing games with Omega Engine! 🎮✨**
