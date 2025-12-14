# Omega Shooter 3D - Quick Start Guide

## What You've Got

A complete, working 3D first-person shooter demo inspired by Wolfenstein 3D and the classic id Software engines!

## Files Created

1. **shooter_3d.cpp** - Complete C++ source code (~500 lines)
2. **shooter_3d.exe** - Compiled Windows executable (ready to run!)
3. **SHOOTER_3D.md** - Full game documentation and features
4. **ID_SOFTWARE_EVOLUTION.md** - Historical context and learning guide

## Quick Start

### Run the Game Right Now!
```bash
cd C:\Users\alecc\omega-engine
.\shooter_3d.exe
```

### Controls
- **WASD** - Move around
- **Arrow Keys** - Look left/right
- **Spacebar** - Shoot
- **Q or ESC** - Quit

### Objective
Eliminate all enemies (red 'E' characters) before they get you!

## What Makes This Special

### Technical Innovation
✅ **Raycasting Engine** - Real 3D from 2D maps (just like Wolfenstein 3D!)
✅ **60 FPS Performance** - Ultra-fast console rendering
✅ **Distance Shading** - Walls fade with depth for realism
✅ **Sprite Enemies** - 2D characters that scale with distance
✅ **Smart AI** - Enemies track and pursue the player
✅ **Full Game Loop** - Health, ammo, score, win/lose conditions

### Educational Value
- Learn raycasting algorithms (DDA)
- Understand 3D projection from 2D space
- See how classic game engines worked
- Foundation for modern 3D graphics
- Pure C++ implementation (no graphics libraries!)

## Comparison to DungeonQuest

| Feature | DungeonQuest | Shooter 3D |
|---------|--------------|------------|
| **Genre** | Roguelike RPG | FPS Shooter |
| **View** | Top-down 2D | First-person 3D |
| **Movement** | Turn-based | Real-time |
| **Combat** | Tactical/Strategic | Action/Reflex |
| **Rendering** | 2D tile grid | 3D raycasting |
| **Inspiration** | Rogue, NetHack | Wolfenstein 3D |

## How to Modify

### Change the Map
Edit the `initMap()` function in shooter_3d.cpp:
- `0` = Empty space (walkable)
- `1` = Gray wall
- `2` = Red wall

### Add More Enemies
Modify `spawnEnemies()` to add enemies at different positions:
```cpp
enemies.push_back(Enemy(x, y));  // Add at position (x, y)
```

### Adjust Difficulty
- Player starting health: `Player() : health(100)`
- Enemy damage: `hitEnemy->health -= 20;`
- Enemy spawn count: Add more in `spawnEnemies()`

## Rebuild from Source

If you modify the code:
```bash
g++ -std=c++17 -O2 -o shooter_3d.exe shooter_3d.cpp
```

## Architecture Overview

### Core Components

1. **Console Renderer**
   - Fast WriteConsoleOutput API
   - Double buffering
   - Color and character buffers

2. **Raycaster Engine**
   - DDA algorithm for wall detection
   - FOV-based column rendering
   - Distance-based shading

3. **Player System**
   - Position and angle
   - Collision detection
   - Health/ammo tracking

4. **Enemy System**
   - AI pursuit behavior
   - Sprite rendering
   - Health management

5. **Game Loop**
   - 60 FPS timing
   - Input handling
   - State updates
   - Rendering

## Next Steps

### Level 1: Enhance This Demo
- [ ] Add pickups (health, ammo)
- [ ] Multiple weapon types
- [ ] Sound effects (console beeps)
- [ ] More enemy types
- [ ] Multiple levels
- [ ] Save/load system

### Level 2: Add Doom Features
- [ ] Height variation (stairs, platforms)
- [ ] Textured walls
- [ ] Floor/ceiling textures
- [ ] Doors and keys
- [ ] Better lighting

### Level 3: Move to True 3D
- [ ] Port to OpenGL
- [ ] Polygon-based rendering
- [ ] True 3D movement (up/down)
- [ ] Full 6 degrees of freedom

### Level 4: Modern Engine
- [ ] PBR materials
- [ ] Dynamic lighting
- [ ] Physics engine
- [ ] Network multiplayer

## Learning Resources

### Understand Raycasting
- **SHOOTER_3D.md** - Technical details of this implementation
- **ID_SOFTWARE_EVOLUTION.md** - Historical context and progression

### Classic Games to Study
1. **Wolfenstein 3D** (1992) - The original
2. **Doom** (1993) - Added height and textures
3. **Quake** (1996) - First true 3D
4. **Call of Duty** (2003+) - Modern evolution (IW Engine)

### Online Resources
- Lode's Raycasting Tutorial
- Game Engine Architecture books
- id Software open source releases
- Graphics programming forums

## Troubleshooting

### Windows SmartScreen Warning
Click "More info" → "Run anyway" (executable is unsigned but safe)

### Compilation Issues
Ensure you have:
- MinGW or MSVC C++ compiler
- C++17 support
- Windows SDK (for console APIs)

### Performance Issues
- Reduce screen resolution in code
- Lower enemy count
- Reduce ray precision (increase step size)

## Share Your Work

Made improvements? Created new levels? Added features?

Consider:
- Sharing your modified version
- Creating a YouTube walkthrough
- Writing a tutorial blog post
- Contributing to Omega Engine

## Philosophy

This demo proves you don't need:
- ❌ A GPU
- ❌ A graphics library
- ❌ Complex math libraries
- ❌ Gigabytes of assets

You just need:
- ✅ Understanding of the algorithm
- ✅ Basic trigonometry
- ✅ Clean code architecture
- ✅ Creativity!

---

## Final Thoughts

**Omega Shooter 3D** demonstrates that the fundamentals of 3D graphics are accessible to everyone. The same concepts that powered Wolfenstein 3D in 1992 are still relevant today in engines like Unreal and Unity - they're just evolved and optimized.

Start here. Understand raycasting. Then build the future.

**Happy shooting! 🎮**

---

**Version**: 1.0
**Created**: 2025
**Engine**: Omega Engine
**Inspiration**: id Software's legendary raycasting engines
