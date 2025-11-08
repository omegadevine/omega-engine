# Performance Optimization Guide

## Overview

This guide covers best practices for optimizing your Omega Engine game.

## Rendering Optimization

### Batching

Batch draw calls to reduce overhead:

```cpp
// Bad: Multiple draw calls
for (auto& sprite : sprites) {
    renderer.drawSprite(sprite);
}

// Good: Batch similar sprites
renderer.beginBatch();
for (auto& sprite : sprites) {
    renderer.addToBatch(sprite);
}
renderer.endBatch();
```

### Culling

Implement frustum culling to skip off-screen objects:

```cpp
Camera& camera = scene.getCamera();
for (auto& entity : entities) {
    if (camera.isInView(entity.getBounds())) {
        entity.render();
    }
}
```

### Texture Atlases

Use texture atlases to reduce texture switches:

```cpp
// Combine multiple textures into one atlas
TextureAtlas atlas("sprites.png");
atlas.addRegion("player", 0, 0, 32, 32);
atlas.addRegion("enemy", 32, 0, 32, 32);
```

### Level of Detail (LOD)

Reduce detail for distant objects:

```cpp
float distance = camera.distanceTo(entity);
if (distance > 1000.0f) {
    entity.setLOD(LOD_LOW);
} else if (distance > 500.0f) {
    entity.setLOD(LOD_MEDIUM);
} else {
    entity.setLOD(LOD_HIGH);
}
```

## Memory Management

### Object Pooling

Reuse objects instead of allocating new ones:

```cpp
class BulletPool {
public:
    Bullet* acquire() {
        if (m_inactive.empty()) {
            return new Bullet();
        }
        Bullet* bullet = m_inactive.back();
        m_inactive.pop_back();
        return bullet;
    }
    
    void release(Bullet* bullet) {
        bullet->reset();
        m_inactive.push_back(bullet);
    }

private:
    std::vector<Bullet*> m_inactive;
};
```

### Asset Management

Load assets efficiently:

```cpp
// Preload common assets at startup
AssetManager::getInstance().preload({
    "player.png",
    "enemy.png",
    "background.png"
});

// Stream large assets asynchronously
AssetManager::getInstance().loadAsync("level_music.ogg", [](Asset* asset) {
    // Play when loaded
});

// Unload unused assets
AssetManager::getInstance().unload("old_level_assets");
```

## ECS Optimization

### Component Caching

Cache frequently accessed components:

```cpp
// Bad: Lookup every frame
void update(float dt) {
    for (auto entity : entities) {
        auto* transform = entity.getComponent<Transform>();
        auto* sprite = entity.getComponent<Sprite>();
        // ...
    }
}

// Good: Cache components
struct CachedEntity {
    Transform* transform;
    Sprite* sprite;
};
std::vector<CachedEntity> cachedEntities;
```

### System Ordering

Order systems by dependency and frequency:

```cpp
// High frequency, no dependencies
systemManager.addSystem<InputSystem>(0);
systemManager.addSystem<PhysicsSystem>(1);

// Medium frequency, depends on physics
systemManager.addSystem<CollisionSystem>(2);

// Low frequency, rendering last
systemManager.addSystem<RenderSystem>(100);
```

## Physics Optimization

### Spatial Partitioning

Use spatial hashing or quadtrees:

```cpp
SpatialHash spatialHash(cellSize);
for (auto& entity : entities) {
    spatialHash.insert(entity);
}

// Query nearby entities
auto nearby = spatialHash.query(position, radius);
```

### Sleep Inactive Bodies

Put stationary physics bodies to sleep:

```cpp
if (body.getVelocity().length() < 0.01f) {
    body.setSleeping(true);
}
```

### Fixed Timestep

Use a fixed timestep for physics:

```cpp
const float FIXED_DT = 1.0f / 60.0f;
float accumulator = 0.0f;

void update(float dt) {
    accumulator += dt;
    while (accumulator >= FIXED_DT) {
        physicsSystem.update(FIXED_DT);
        accumulator -= FIXED_DT;
    }
}
```

## Profiling

Use the built-in profiler to identify bottlenecks:

```cpp
Profiler::getInstance().beginSection("Update");
updateGame(dt);
Profiler::getInstance().endSection("Update");

Profiler::getInstance().beginSection("Render");
renderGame();
Profiler::getInstance().endSection("Render");

// Print results
auto stats = Profiler::getInstance().getStats();
std::cout << "Update: " << stats["Update"] << "ms" << std::endl;
std::cout << "Render: " << stats["Render"] << "ms" << std::endl;
```

## Target Framerates

Set appropriate target framerates:

```cpp
// 60 FPS for desktop
engine.setTargetFPS(60);

// 30 FPS for mobile
engine.setTargetFPS(30);

// Uncapped for benchmarking
engine.setTargetFPS(0);
```

## Multi-threading

Parallelize independent systems:

```cpp
std::thread physicsThread([&]() {
    physicsSystem.update(dt);
});

std::thread aiThread([&]() {
    aiSystem.update(dt);
});

physicsThread.join();
aiThread.join();

// Render on main thread
renderSystem.update(dt);
```

## Quality Settings

Provide quality presets:

```cpp
enum QualityPreset {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA
};

void applyQuality(QualityPreset preset) {
    switch (preset) {
        case LOW:
            renderer.setShadowQuality(SHADOW_LOW);
            renderer.setParticleLimit(100);
            postProcess.setEnabled(false);
            break;
        case MEDIUM:
            renderer.setShadowQuality(SHADOW_MEDIUM);
            renderer.setParticleLimit(500);
            postProcess.setEnabled(true);
            break;
        // ...
    }
}
```

## Monitoring

Track performance metrics:

```cpp
struct PerformanceMetrics {
    float fps;
    float frameTime;
    int drawCalls;
    int triangles;
    size_t memoryUsed;
    int entityCount;
};

PerformanceMetrics getMetrics() {
    return {
        Profiler::getFPS(),
        Profiler::getFrameTime(),
        Renderer::getDrawCalls(),
        Renderer::getTriangleCount(),
        Memory::getUsed(),
        ECS::getEntityCount()
    };
}
```
