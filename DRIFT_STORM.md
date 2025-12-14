# DRIFT STORM - First-Person Drift Racing Game

A console-based first-person drift racing game for the Omega Engine project, inspired by Genki's Shutoko Battle series, Initial D Arcade Stage, and Japanese street racing culture.

## Overview

DRIFT STORM uses a raycasting pseudo-3D engine (similar to the approach used in early id Software games like Wolfenstein 3D) to create an immersive first-person driving experience. The game features:

- **Cockpit-view driving** with depth-projected road rendering
- **Advanced drift physics** with weight transfer and tire grip simulation
- **Open-world circuit** combining highway and touge (mountain pass) sections
- **Traffic system** with AI vehicles to weave through
- **Shutoko-style HUD** with speedometer, tachometer, and drift scoring

## Controls

| Key | Action |
|-----|--------|
| W / Up Arrow | Accelerate |
| S / Down Arrow | Brake |
| A / Left Arrow | Steer Left |
| D / Right Arrow | Steer Right |
| SPACE | Handbrake (initiate/maintain drift) |
| SHIFT | Clutch Kick (rev up, break traction) |
| Q / ESC | Quit |

## Drift Techniques

### Initiating a Drift

1. **Handbrake Entry**: Approach a corner at speed, tap SPACE while turning to break rear traction
2. **Clutch Kick**: Hold SHIFT to rev the engine, release while turning to spin the rear wheels
3. **Power Over**: In tight corners, apply full throttle (W) while steering to overwhelm rear grip

### Maintaining a Drift

- **Countersteer**: Steer in the opposite direction of the slide to balance the car
- **Throttle Control**: Modulate throttle to control drift angle - more throttle = wider angle
- **Weight Transfer**: Lift off throttle briefly to shift weight forward, then reapply

### Drift Scoring

- **Angle**: Higher drift angles (up to 90°) score more points
- **Speed**: Faster drifts multiply your score
- **Duration**: Longer drifts build a combo multiplier (up to 5x)
- **Chain Drifts**: Link multiple corners for maximum score

## Track Layout

The open-world circuit features distinct zones:

### Highway Section (~4km)
- Wide, multi-lane road
- Gentle curves ideal for high-speed drifting
- Traffic to navigate around
- Good for building speed before mountain section

### Touge Entry
- Transition from highway to mountain
- Road narrows, gradient increases
- Begin practicing technical drifts

### Touge Hairpins
- Tight, technical switchbacks
- Guardrail-lined narrow road
- Multiple consecutive hairpin turns
- High-skill drift zone

### Tunnel Section
- Enclosed tunnel with reduced visibility
- No traffic, smooth curves
- Good for practicing lines

### Touge Downhill
- Fast, flowing corners
- Descending gradient
- High-speed drift opportunities
- Return to highway

## Physics System

### Car Specifications (Tuned for drift)
- **Mass**: 1350 kg (similar to AE86/S13)
- **Wheelbase**: 2.4m
- **Front Grip**: 1.1 (high for precise steering)
- **Rear Grip**: 0.95 (slightly lower for easier drift initiation)
- **6-Speed Manual** (auto-shift enabled)
- **Redline**: 8500 RPM

### Weight Transfer
- Acceleration shifts weight rearward
- Braking shifts weight forward
- Lateral forces during cornering affect left/right distribution
- Weight affects tire grip (more weight = more grip)

### Tire Model
- Simplified Pacejka-style tire force calculation
- Tire temperature affects grip
- Optimal temp: 85-95°C
- Overheating (>100°C) reduces grip

### Drift Detection
- Drift angle = difference between car heading and velocity direction
- Drift registered when angle > 10° and speed > 10 m/s
- Drift scoring based on angle × speed × duration × combo

## Technical Details

### Rendering
- **Resolution**: 160×50 characters
- **Frame Rate**: ~60 FPS rendering, 120+ physics updates/sec
- **Raycasting**: Pseudo-3D road projection with depth mapping
- **Z-buffering**: Proper occlusion for traffic rendering

### Console Optimization
- Uses Windows Console API `WriteConsoleOutput` for fast buffer swaps
- Custom font size (6×10 pixels) for higher resolution
- Double-buffered rendering to eliminate flicker

## Building

### With MinGW/g++
```batch
g++ -O2 -std=c++17 drift_storm.cpp -o drift_storm.exe
```

### With Visual Studio
```batch
cl /EHsc /O2 /std:c++17 drift_storm.cpp /Fe:drift_storm.exe
```

## Future Expansion Ideas

### Near-term
- [ ] Rival car to race against (Shutoko-style)
- [ ] Multiple car selection with different handling
- [ ] Ghost replay system
- [ ] Sound system (engine, tire squeal, wind)

### Medium-term
- [ ] Additional track sections (wangan highway, industrial area)
- [ ] Time attack mode with split times
- [ ] Car tuning (power, suspension, gear ratios)
- [ ] Weather effects (rain, night mode)

### Long-term
- [ ] Online leaderboards
- [ ] Full career mode
- [ ] SDL2/OpenGL port for true graphics
- [ ] Controller support

## Inspirations

- **Shutoko Battle / Tokyo Xtreme Racer** (Genki) - Highway racing, minimalist HUD, rival system
- **Initial D Arcade Stage** (Sega) - Touge racing, drift mechanics, technical mountain passes
- **Wangan Midnight Maximum Tune** (Namco) - High-speed highway racing
- **Assetto Corsa** - Physics model inspiration
- **Forza Horizon** - Drift scoring system concept

## Credits

Part of the **Omega Engine** project - A minimal-from-scratch 2D game engine.

The raycasting approach is inspired by the id Software technique used in Wolfenstein 3D, adapted here for driving game perspective.

## License

MIT License - See LICENSE file in the project root.
