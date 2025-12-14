# Drift Racer - Driving Game Demo

A console-based driving game demo for the Omega Engine project, designed as a foundation for a full drift simulator.

## Features

### Core Mechanics
- **Car Physics**: Realistic(ish) 2D car physics with acceleration, braking, and steering
- **Drift System**: Use the handbrake to break traction and initiate drifts
- **Grip Simulation**: Different grip levels for normal driving vs drifting
- **Momentum**: Car velocity and direction are tracked separately, enabling realistic slides

### Gameplay
- **Oval Track**: Race around an oval track with barriers and checkpoints
- **Lap Timing**: Complete lap times tracked with best lap recording
- **Checkpoints**: Must pass all checkpoints to complete a lap
- **Visual Feedback**: Car changes color when drifting, tire marks appear

## Controls

| Key | Action |
|-----|--------|
| W / Up Arrow | Accelerate |
| S / Down Arrow | Brake / Reverse |
| A / Left Arrow | Steer Left |
| D / Right Arrow | Steer Right |
| SPACE | Handbrake (initiate drift!) |
| R | Reset car position |
| Q / ESC | Quit game |

## How to Play

1. Start the game and press any key
2. Drive around the track counter-clockwise
3. Pass through all blue checkpoint lines
4. Cross the yellow finish line to complete a lap
5. Use the handbrake (SPACE) in corners to drift!
6. Try to beat your best lap time

## Building

### Using Visual Studio Developer Command Prompt
```batch
cl /EHsc /O2 drift_racer.cpp /Fe:drift_racer.exe
```

### Using MinGW/g++
```batch
g++ -O2 drift_racer.cpp -o drift_racer.exe
```

### Using the batch builder
```batch
build_drift_racer.bat
```

## Physics Parameters (for tuning)

The `Car` class contains tuneable physics parameters:

```cpp
mass = 1200.0;       // Vehicle mass (kg)
enginePower = 8000.0; // Engine force
brakePower = 12000.0; // Braking force
maxSpeed = 50.0;      // Maximum velocity
grip = 0.95;          // Normal tire grip (0-1)
driftGrip = 0.6;      // Grip while drifting
steerSpeed = 3.5;     // Steering responsiveness
dragCoeff = 0.4;      // Air resistance
rollingResist = 0.01; // Rolling friction
```

## Expansion Ideas

This demo is designed to be expanded into a full drift simulator. Ideas include:

### Near-term Enhancements
- [ ] Multiple track layouts (figure-8, street circuit, mountain pass)
- [ ] Drift scoring system (angle × speed × duration)
- [ ] Ghost car (race against your best lap)
- [ ] Sound effects (engine, tire squeal)

### Medium-term Features
- [ ] Multiple car types with different handling
- [ ] Car upgrades (engine, tires, suspension)
- [ ] Weather effects (rain reduces grip)
- [ ] Day/night cycle

### Long-term Goals
- [ ] AI opponents
- [ ] Career mode with progression
- [ ] Track editor
- [ ] Multiplayer (split-screen or network)
- [ ] Port to SDL2/OpenGL for better graphics

## Technical Notes

### Console Rendering
- Uses Windows Console API for fast double-buffered rendering
- Achieves ~60 FPS with WriteConsoleOutput
- Thread-safe buffer updates with mutex

### Physics Model
- Simplified arcade-style physics
- Separate forward/lateral tire forces
- Handbrake reduces lateral grip for drifting
- Speed-dependent steering (harder to turn at high speed)

### Track System
- Grid-based track with different tile types
- Collision detection with barriers
- Grass slows the car down
- Checkpoint system for lap validation

## Credits

Part of the **Omega Engine** project - A minimal-from-scratch 2D game engine.

## License

MIT License - See LICENSE file in the project root.
