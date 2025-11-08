```markdown
# omega-engine

A minimal-from-scratch game engine scaffold.

This repository contains the initial scaffolding for a cross-platform game engine:
- Language: C++17
- Build system: CMake
- Windowing & input: SDL2
- Renderer: OpenGL 3.3 Core
- License: MIT

Goals
- Small, well-documented codebase for learning and building a modular engine.
- Start with window creation and a clear-screen example, then incrementally add renderer features, an ECS, scripting, physics, audio, and tools.

Getting started (development)
1. Install dependencies:
   - Linux (Ubuntu): sudo apt-get install build-essential cmake libsdl2-dev libgl1-mesa-dev
   - macOS: brew install cmake sdl2
   - Windows: Install MSVC, CMake, and use vcpkg for SDL2: vcpkg install sdl2:x64-windows
2. mkdir build && cd build
3. cmake .. && cmake --build .

Project Layout
- src/            — engine source (main entry, renderer, shaders)
- assets/         — runtime assets (textures, shaders, audio)
- external/       — third-party submodules / vendored libs (optional)
- tests/          — unit / integration tests
- docs/           — design notes and docs

Roadmap (initial)
- [x] Project scaffolding & CI
- [x] Window + input subsystem (SDL2)
- [x] OpenGL initialization (3.3 Core)
- [x] Renderer: clear screen with animated colors
- [x] Shader system: compile, link, error handling
- [ ] Sprite rendering system
- [ ] Texture loading
- [ ] Basic ECS
- [ ] Asset pipeline
- [ ] Scripting (Lua)
- [ ] Physics integration (optional)

Features Implemented
- ✅ Cross-platform window creation (SDL2)
- ✅ OpenGL 3.3 Core context
- ✅ Basic renderer with clear color
- ✅ GLSL shader compilation system
- ✅ Animated background demo
- ✅ Security scanning (CodeQL)
- ✅ Automated backups
- ✅ CI/CD for Linux, macOS, and Windows

Contributing
See CONTRIBUTING.md

License
MIT — see LICENSE
```
