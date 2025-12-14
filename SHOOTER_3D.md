# OMEGA SHOOTER 3D

A classic Wolfenstein 3D-style raycasting first-person shooter demo built on the Omega Engine.

## Overview

This demo showcases a 3D shooter game using raycasting rendering techniques, similar to the original id Software engines that powered Wolfenstein 3D and later evolved into the engines for Doom, Quake, and eventually the IW engine (Call of Duty).

## Features

### Raycasting Engine
- **Real-time 3D rendering** using 2D raycasting
- **60-degree field of view** for immersive gameplay
- **Distance-based shading** for depth perception
- **Texture-like wall rendering** with different wall types
- **Sprite-based enemies** that scale with distance

### Gameplay Elements
- **First-person movement** (WASD controls)
- **360-degree rotation** (Arrow keys)
- **Shooting mechanics** (Spacebar to fire)
- **Enemy AI** that tracks and pursues the player
- **Health and ammo system**
- **Score tracking**

### Technical Features
- **Ultra-fast console rendering** using WriteConsoleOutput
- **Double buffering** for smooth visuals
- **~60 FPS performance**
- **Efficient ray-wall collision detection**
- **Line-of-sight shooting system**

## Controls

| Key | Action |
|-----|--------|
| W | Move forward |
| S | Move backward |
| A | Strafe left |
| D | Strafe right |
| ← | Rotate left |
| → | Rotate right |
| Space | Shoot |
| Q / ESC | Quit game |

## How to Play

1. **Run the executable**: `shooter_3d.exe`
2. **Objective**: Eliminate all enemies in the level
3. **Combat**: 
   - Aim at enemies using arrow keys
   - Shoot with spacebar (costs 1 ammo per shot)
   - Each enemy takes multiple hits to defeat
4. **Survival**: Avoid letting enemies get too close or they'll damage you
5. **Victory**: Eliminate all enemies before your health runs out

## Building from Source

```bash
g++ -std=c++17 -O2 -o shooter_3d.exe shooter_3d.cpp
```

## Technical Details

### Raycasting Algorithm
The rendering engine uses the classic DDA (Digital Differential Analyzer) raycasting algorithm:
1. For each screen column, cast a ray from the player's position
2. March the ray through the map until it hits a wall
3. Calculate wall height based on distance (perspective projection)
4. Render ceiling, wall, and floor for that column

### Enemy Rendering
Enemies are rendered as sprites using:
- Angular position relative to player view direction
- Distance-based scaling for size
- Character variation based on distance ('e', 'E', '@')

### Performance Optimizations
- **Single API call rendering**: WriteConsoleOutput instead of individual gotoxy calls
- **Lock-based buffer protection**: Thread-safe rendering system
- **Distance culling**: Only render objects within visible range
- **Hidden cursor**: Eliminates cursor blink overhead

## Map Format

The game uses a 2D grid map where:
- `0` = Empty space (walkable)
- `1` = Standard wall (gray)
- `2` = Special wall (red)

You can easily modify the map in the `initMap()` function to create new levels.

## Comparison to Classic Engines

### Wolfenstein 3D (1992)
- ✅ Similar raycasting technique
- ✅ 2.5D rendering (no height variation)
- ✅ Sprite-based enemies
- ⚡ Omega Engine uses modern C++ and console rendering

### id Tech / IW Engine Evolution
- **Wolfenstein 3D** → Raycasting, grid-based
- **Doom** → Added height variation, texture mapping
- **Quake** → Full 3D rendering with BSP
- **IW Engine** → Advanced physics, networking, modern graphics
- **Omega Engine** → Educational, accessible, console-based

## Future Enhancements

Potential additions for this demo:
- [ ] Multiple weapon types
- [ ] Pickups (health, ammo, keys)
- [ ] Doors and interactive objects
- [ ] Multiple levels
- [ ] Different enemy types
- [ ] Textured walls (using ASCII art)
- [ ] Sound effects (console beeps)
- [ ] Minimap display

## Credits

Built with the **Omega Engine** framework
Inspired by **id Software**'s revolutionary raycasting technology

---

**Version**: 1.0
**Date**: 2025
**Engine**: Omega Engine
