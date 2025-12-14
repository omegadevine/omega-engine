# Omega Engine Demo Showcase

## Two Complete Game Demos

The Omega Engine now includes **two fully playable game demos** that showcase different aspects of game development:

---

## 🏰 DungeonQuest - Roguelike RPG

```
╔════════════════════════════════════════════╗
║  #  #  #  #  #  #  #  #  #  #  #  #  #  # ║
║  #  .  .  .  .  .  .  .  .  .  .  .  .  # ║
║  #  .  @  .  .  E  .  .  $  .  .  !  .  # ║
║  #  .  .  █  █  █  .  .  .  █  █  █  .  # ║
║  #  .  .  .  .  .  .  E  .  .  .  .  .  # ║
║  #  .  $  .  .  █  █  █  █  █  .  .  E  # ║
║  #  .  .  .  .  .  .  .  .  .  .  .  .  # ║
║  #  #  #  #  #  #  #  #  #  #  #  #  #  # ║
╚════════════════════════════════════════════╝

Health: 100 | Mana: 50 | Gold: 0
Level 1 | Floor: 1
```

### Features
- **Genre**: Traditional Roguelike
- **View**: Top-down 2D
- **Combat**: Turn-based tactical
- **Mechanics**: 
  - Procedural generation
  - Permadeath
  - Character progression
  - Inventory system
  - Magic spells
  - Monster AI

### Inspired By
- Rogue (1980)
- NetHack (1987)
- Dungeon Crawl Stone Soup

### Perfect For Learning
- Procedural generation algorithms
- Turn-based game logic
- Inventory/equipment systems
- Roguelike mechanics

---

## 🔫 Omega Shooter 3D - First-Person Shooter

```
     ████████████████████████████
   ██                            ██
  █                                █
 █          ████        ████        █
█          ██  ██      ██  ██        █
█          ██  ██      ██  ██        █
█          ██  ██      ██  ██        █
█           ████        ████         █
█                                    █
█              ▓▓▓▓                  █
█             ▓▓ E ▓▓                █
█              ▓▓▓▓                  █
█                                    █
 █                                  █
  █                                █
   ██                            ██
     ████████████████████████████

Health: 85 | Ammo: 47 | Score: 200
WASD: Move | Arrows: Look | Space: Shoot
```

### Features
- **Genre**: First-Person Shooter
- **View**: 3D Raycasting (first-person)
- **Combat**: Real-time action
- **Mechanics**:
  - 3D raycasting engine
  - Distance-based shading
  - Sprite-based enemies
  - Enemy AI pursuit
  - Line-of-sight shooting
  - Health/ammo system

### Inspired By
- Wolfenstein 3D (1992)
- Doom (1993)
- Classic id Software engines

### Perfect For Learning
- Raycasting algorithms (DDA)
- 3D projection from 2D
- Real-time rendering
- FPS game mechanics

---

## Side-by-Side Comparison

| Aspect | DungeonQuest | Omega Shooter 3D |
|--------|--------------|------------------|
| **Released** | ✅ Multiple versions | ✅ Version 1.0 |
| **Genre** | Roguelike RPG | FPS Shooter |
| **Perspective** | Top-down | First-person |
| **Rendering** | 2D tile-based | 3D raycasting |
| **Movement** | Grid-based, turn-based | Smooth, real-time |
| **Combat** | Tactical, turn-based | Action, real-time |
| **Enemies** | Various types with AI | Simple pursuit AI |
| **Progression** | Levels, XP, equipment | Score-based |
| **Map** | Procedural generation | Static level design |
| **Complexity** | Medium-High | Low-Medium |
| **Code Lines** | ~2000+ | ~500 |
| **Learning Focus** | Game systems | Graphics algorithms |

---

## When to Use Each as a Learning Tool

### Start with DungeonQuest if you want to learn:
- ✅ Game state management
- ✅ Turn-based mechanics
- ✅ Inventory and equipment systems
- ✅ Procedural generation
- ✅ Complex AI behaviors
- ✅ RPG mechanics (XP, stats, magic)

### Start with Omega Shooter 3D if you want to learn:
- ✅ 3D graphics fundamentals
- ✅ Raycasting algorithms
- ✅ Real-time game loops
- ✅ Physics and collision (3D)
- ✅ First-person controls
- ✅ Distance-based rendering

---

## Technical Architecture

### DungeonQuest Architecture
```
┌─────────────────────────────────┐
│      Console Renderer           │
│   (Fast double-buffering)       │
├─────────────────────────────────┤
│      Game State Manager         │
│   (Turn processor, events)      │
├─────────────────────────────────┤
│      Dungeon Generator          │
│   (BSP, rooms, corridors)       │
├─────────────────────────────────┤
│      Entity System              │
│   (Player, monsters, items)     │
├─────────────────────────────────┤
│      Combat System              │
│   (Stats, damage, magic)        │
├─────────────────────────────────┤
│      Input Handler              │
│   (Keyboard, commands)          │
└─────────────────────────────────┘
```

### Omega Shooter 3D Architecture
```
┌─────────────────────────────────┐
│      Console Renderer           │
│   (WriteConsoleOutput API)      │
├─────────────────────────────────┤
│      Raycaster Engine           │
│   (DDA algorithm, FOV)          │
├─────────────────────────────────┤
│      Player System              │
│   (Position, angle, health)     │
├─────────────────────────────────┤
│      Enemy System               │
│   (AI, sprites, collision)      │
├─────────────────────────────────┤
│      Game Loop                  │
│   (60 FPS, input, update)       │
├─────────────────────────────────┤
│      Input Handler              │
│   (WASD, arrows, shoot)         │
└─────────────────────────────────┘
```

---

## Running the Demos

### DungeonQuest
```bash
cd C:\Users\alecc\omega-engine
.\dungeon_quest_v5_balanced.exe
```

### Omega Shooter 3D
```bash
cd C:\Users\alecc\omega-engine
.\shooter_3d.exe
```

---

## Evolution Path

### From Simple to Complex

```
1. [START] Omega Shooter 3D
   └─> Learn raycasting basics
   └─> Understand 3D projection
   └─> Master real-time loops

2. [NEXT] DungeonQuest
   └─> Complex game systems
   └─> Procedural generation
   └─> Turn-based strategy

3. [ADVANCED] Hybrid Game
   └─> Combine both techniques
   └─> 3D dungeon crawler?
   └─> Real-time roguelike?

4. [MASTERY] Full Engine Game
   └─> Use Omega Engine proper
   └─> OpenGL rendering
   └─> Complete production game
```

---

## What's Next?

### Enhance DungeonQuest
- More spell types
- Boss fights
- Achievement system
- Multiple character classes
- Online leaderboards

### Enhance Omega Shooter 3D
- Textured walls
- Multiple weapons
- More enemy types
- Level progression
- Boss battles

### Combine the Concepts
Create a **3D roguelike** using raycasting + procedural generation!

---

## Credits

Both demos built with the **Omega Engine** framework.

- **DungeonQuest**: Inspired by classic roguelikes
- **Omega Shooter 3D**: Inspired by id Software's Wolfenstein 3D

---

## Philosophy

> "Great game engines aren't just about rendering triangles. They're about empowering creators to build **any kind of game** they can imagine."

The Omega Engine proves this by supporting:
- ✅ 2D sprite-based games
- ✅ 2D tile-based games  
- ✅ 3D raycasting games
- ✅ Turn-based games
- ✅ Real-time games

**All with the same core philosophy: Fast, clean, educational code.**

---

**Pick your adventure. Start coding. Build games. 🎮**
