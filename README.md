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
- [ ] Particle system
- [ ] UI system (buttons, labels, panels)
- [ ] Scripting (Lua)
- [ ] Physics integration (Box2D)
- [ ] Tilemap renderer
- [ ] Save/Load system

Demo Application
The included demo showcases:
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
- Input handling across multiple scenes
- Pause menu with resume/restart/quit options

Contributing
See CONTRIBUTING.md

License
MIT — see LICENSE
```
