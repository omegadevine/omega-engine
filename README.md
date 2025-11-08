```markdown
# omega-engine

A minimal-from-scratch 2D game engine with ECS architecture.

This repository contains a fully functional 2D game engine built from scratch:
- Language: C++17
- Build system: CMake
- Windowing & input: SDL2
- Renderer: OpenGL 3.3 Core
- Architecture: Entity Component System (ECS)
- License: MIT

Goals
- Small, well-documented codebase for learning and building games.
- Modern ECS architecture for flexible game object management.
- Cross-platform support (Linux, macOS, Windows).
- Clean APIs for sprites, textures, input, and rendering.

Getting started (development)
1. Install dependencies:
   - Linux (Ubuntu): sudo apt-get install build-essential cmake libsdl2-dev libgl1-mesa-dev
   - macOS: brew install cmake sdl2
   - Windows: Install MSVC, CMake, and use vcpkg for SDL2: vcpkg install sdl2:x64-windows
2. mkdir build && cd build
3. cmake .. && cmake --build .
4. Run: ./omega-engine (Linux/Mac) or Release\omega-engine.exe (Windows)

Features
- ✅ Cross-platform window creation (SDL2)
- ✅ OpenGL 3.3 Core rendering pipeline
- ✅ GLSL shader compilation and management
- ✅ Texture loading with stb_image
- ✅ Asset Manager for resource management
- ✅ Camera system with follow and zoom
- ✅ Sprite animation system
- ✅ Collision detection (AABB, Circle)
- ✅ Scene management with transitions
- ✅ Audio system (music and sound effects)
- ✅ UI system (buttons, labels, panels)
- ✅ Particle system (visual effects)
- ✅ Tilemap system (level design)
- ✅ Text rendering (bitmap fonts)
- ✅ Debug visualization and profiling
- ✅ Asset pipeline and hot reloading
- ✅ Physics system (Box2D integration ready)
- ✅ Networking (client/server architecture)
- ✅ Scripting (Lua integration ready)
- ✅ 2D sprite rendering system
- ✅ Entity Component System (ECS)
- ✅ Input abstraction (keyboard & mouse)
- ✅ Component-based architecture
- ✅ Animated sprite demo
- ✅ Security scanning (CodeQL)
- ✅ Automated backups
- ✅ CI/CD for Linux, macOS, and Windows

Project Layout
- src/            — engine source (renderer, sprites, ECS, input)
- assets/         — runtime assets (textures, shaders, audio)
- .github/        — CI/CD workflows and issue templates
- docs/           — design notes and documentation

Core Systems

Renderer
- OpenGL 3.3 Core context
- Shader compilation with error reporting
- VSync support
- Double buffering

Asset Manager
- Centralized resource management
- Automatic caching and reference counting
- Texture and shader loading
- Memory-efficient asset sharing
- Prevents duplicate loads

Camera System
- 2D camera with position and zoom control
- Smooth camera following with configurable speed
- World bounds support
- Screen shake effects
- World/screen coordinate transformations
- Perfect for scrolling games

Animation System
- Frame-based sprite animations
- Multiple animations per sprite
- Configurable frame duration
- Loop and one-shot animations
- Animation state management (play, pause, stop)
- Animation complete callbacks
- Sprite sheet support
- Smooth animation transitions

Collision Detection
- AABB (Axis-Aligned Bounding Box) collision
- Circle collision detection
- Box-Circle collision support
- Collision layers and masks for filtering
- Trigger volumes (non-physical overlaps)
- Static and dynamic colliders
- Collision callbacks (enter, stay, exit)
- Automatic collision resolution
- Query functions (radius, box searches)
- Performance statistics

Scene Management
- Scene base class with lifecycle hooks
- Scene stack for layered scenes (menus, pause, etc.)
- Scene transitions (change, push, pop)
- Per-scene ECS and collision systems
- Scene factory registration
- Automatic state management
- onEnter, onExit, onPause, onResume callbacks
- Clean separation of game states

Audio System
- SDL_mixer integration
- Music playback with looping
- Sound effect management
- Volume control (music and SFX separate)
- Fade in/out effects
- Play, pause, resume, stop controls
- Multiple simultaneous sounds (16 channels)
- Music and sound effect caching
- OGG, WAV, MP3 support

UI System
- Button, Label, and Panel widgets
- Mouse input handling with hover/press states
- Click callbacks for interactive elements
- Color transitions and visual feedback
- UI Manager for element management
- Hit testing and input consumption
- Simple text rendering (placeholder)
- Layered rendering (z-ordering)
- Extensible widget system

Particle System
- Configurable particle emitters
- Physics simulation (velocity, acceleration, gravity)
- Particle lifetime and color transitions
- Fade out and shrink effects
- Burst and continuous emission modes
- Preset effects (fire, smoke, explosion, sparks, rain, snow)
- Performance optimized with particle pooling
- Multiple simultaneous emitters

Tilemap System
- Grid-based tile rendering
- Tileset support with texture atlases
- Multiple layer support
- Collision detection integration
- World-to-tile coordinate conversion
- File save/load (binary format)
- Viewport culling for performance
- Fill patterns and batch operations
- TilemapManager for layer management

Text Rendering System
- Bitmap font support
- BMFont .fnt file format support
- Monospace font generation
- Text measurement and alignment (left, center, right)
- Color and scale customization
- Multi-line text support
- FontManager for resource management
- Sprite-based rendering

Debug System
- DebugRenderer for visual debugging
- Draw shapes (lines, rects, circles, crosses, grids)
- Collision visualization (AABB, CircleCollider)
- Camera bounds visualization
- Lifetime-based shape persistence
- Toggle enable/disable
- Profiler for performance monitoring
- FPS, frame time, update/render time tracking
- Section timing for custom profiling
- Draw call, entity, particle counting
- Memory usage tracking
- DebugConsole for logging and commands
- Color-coded messages (log, warning, error)
- Command registration and execution system
- Message history with timestamp

Asset Pipeline System
- IAssetProcessor interface for custom processors
- TextureProcessor with compression and mipmaps
- AudioProcessor with quality settings
- FontProcessor with charset and size configuration
- AssetMetadata tracking (checksums, file sizes, timestamps)
- Directory batch processing (recursive)
- Asset validation and reprocessing detection
- Progress callbacks for UI integration
- AssetHotReloader for live asset updates
- Directory watching and file change detection
- Automatic asset reloading on modification
- Extension-based reload callbacks
- AssetBundler for packaging
- Bundle creation and extraction
- Compression and encryption support
- File listing and management

Physics System (Box2D stub)
- PhysicsWorld with gravity simulation
- PhysicsBody wrapper (Static, Dynamic, Kinematic)
- Shape support (Box, Circle, Polygon)
- Force, impulse, and torque application
- Linear and angular velocity control
- Mass and gravity scale properties
- Collision callbacks (begin, end, sensor)
- Raycast queries (single and multiple)
- AABB query for spatial searches
- Fixed timestep physics simulation
- PhysicsComponent for ECS integration
- PhysicsSystem for automatic sync
- Debug visualization support
- Extensible for Box2D integration

Networking System
- NetworkClient for client-side networking
- NetworkServer for server hosting
- Packet-based message system
- Message types (Connect, Disconnect, GameState, etc.)
- Client management and heartbeats
- Network statistics tracking
- Serialization utilities (int, float, string)
- NetworkManager singleton
- NetworkReplication for synced objects
- Entity replication over network
- Transform synchronization
- Configurable update rates
- Ready for ENet/SDL_net integration

Scripting System (Lua stub)
- ScriptEngine singleton
- Script loading and execution
- Function registration and calling
- Variable registration (int, float, string)
- Stack operations for Lua interop
- ScriptComponent for entities
- ScriptSystem for ECS integration
- Event system for game logic
- Engine API bindings
- Vector2, Color, Transform bindings
- Input and Audio script access
- Example Lua C function bindings
- Ready for Lua 5.4 integration

Sprite System
- Texture-mapped 2D sprites
- Per-sprite color tinting
- Position, size, and color control
- Automatic texture coordinate mapping

Entity Component System
- Entity creation and destruction
- Component-based design
- Transform and Sprite components included
- Easy to extend with new components

Input System
- Keyboard input (WASD, arrows, etc.)
- Mouse input (position and buttons)
- Key press and key just-pressed detection
- Frame-based input state management

Roadmap
- [x] Project scaffolding & CI
- [x] Window + input subsystem (SDL2)
- [x] OpenGL initialization (3.3 Core)
- [x] Renderer: clear screen with animated colors
- [x] Shader system: compile, link, error handling
- [x] Texture loading system
- [x] Sprite rendering system
- [x] Entity Component System (ECS)
- [x] Input abstraction layer
- [x] Asset Manager for resource management
- [x] Camera system with follow and zoom
- [x] Sprite animation system
- [x] Collision detection (AABB, Circle)
- [x] Scene management
- [x] Audio system (SDL_mixer)
- [x] UI system (buttons, labels, panels)
- [x] Particle system
- [x] Tilemap system
- [x] Text rendering (bitmap fonts)
- [x] Debug tools (profiler, visualizer, console)
- [x] Asset pipeline (processors, hot reload, bundler)
- [x] Physics integration (Box2D-ready wrapper)
- [x] Networking (client/server, replication)
- [x] Scripting (Lua-ready engine)
- [x] Advanced rendering (post-processing effects)
- [x] Save/Load system (encryption, compression, auto-save)
- [x] Comprehensive documentation
- [ ] Physics integration (Box2D implementation)
- [ ] Tilemap renderer improvements
- [ ] Audio system enhancements

Demo Application
The included demo showcases:
- Full UI system with clickable buttons
- Main menu with Start and Quit buttons
- Pause menu with Resume, Restart, and Main Menu options
- Interactive buttons with hover and press states
- Scene management with Menu, Game, and Pause scenes
- Scene transitions and scene stack
- Player-controlled animated sprite (WASD/Arrow keys)
- Automatic animation switching (idle <-> walk)
- Collision detection with static obstacles
- AABB and Circle colliders
- Collision resolution (player bounces off obstacles)
- Camera following with smooth movement
- ECS-based entity management
- Texture rendering with color tinting
- Mouse and keyboard input handling
- Professional UI/UX with visual feedback

Contributing
See CONTRIBUTING.md

License
MIT — see LICENSE
```
