#ifndef OMEGA_COLLISION_H
#define OMEGA_COLLISION_H

#include "Sprite.h"
#include "ECS.h"
#include <functional>

// Collision shape types
enum class ColliderType {
    Box,    // AABB (Axis-Aligned Bounding Box)
    Circle
};

// Collider component
struct Collider : public Component {
    ColliderType type;
    Vector2 offset;       // Offset from entity position
    Vector2 size;         // For Box: width/height, For Circle: x=radius
    int layer;            // Collision layer (bitmask)
    int mask;             // Which layers this collides with (bitmask)
    bool isTrigger;       // If true, detects but doesn't resolve collision
    bool isStatic;        // Static colliders don't move
    
    // Callbacks
    std::function<void(Entity)> onCollisionEnter;
    std::function<void(Entity)> onCollisionStay;
    std::function<void(Entity)> onCollisionExit;
    
    Collider() 
        : type(ColliderType::Box)
        , offset(0, 0)
        , size(32, 32)
        , layer(1)
        , mask(0xFFFFFFFF)
        , isTrigger(false)
        , isStatic(false) {}
};

// Collision information
struct CollisionInfo {
    Entity entityA;
    Entity entityB;
    Vector2 normal;       // Collision normal (from A to B)
    float penetration;    // How deep the collision is
    bool isTrigger;
    
    CollisionInfo() 
        : entityA(0), entityB(0), normal(0, 0), penetration(0), isTrigger(false) {}
};

// Collision detection system
class CollisionSystem {
public:
    CollisionSystem(ECS* ecs);
    ~CollisionSystem() = default;
    
    // Update collision detection (call each frame)
    void update();
    
    // Check collision between two entities
    bool checkCollision(Entity a, Entity b, CollisionInfo* outInfo = nullptr);
    
    // Raycast
    bool raycast(const Vector2& origin, const Vector2& direction, float maxDistance, 
                 Entity* hitEntity = nullptr, Vector2* hitPoint = nullptr);
    
    // Query functions
    std::vector<Entity> getEntitiesInRadius(const Vector2& center, float radius);
    std::vector<Entity> getEntitiesInBox(const Vector2& center, const Vector2& size);
    
    // Statistics
    int getCollisionCount() const { return m_collisionCount; }
    int getChecksPerformed() const { return m_checksPerformed; }

private:
    bool checkAABB(const Vector2& posA, const Vector2& sizeA, 
                   const Vector2& posB, const Vector2& sizeB,
                   CollisionInfo* outInfo);
                   
    bool checkCircle(const Vector2& posA, float radiusA,
                     const Vector2& posB, float radiusB,
                     CollisionInfo* outInfo);
                     
    bool checkBoxCircle(const Vector2& boxPos, const Vector2& boxSize,
                        const Vector2& circlePos, float radius,
                        CollisionInfo* outInfo);
    
    void resolveCollision(Entity a, Entity b, const CollisionInfo& info);
    
    Vector2 getEntityPosition(Entity entity);
    
    ECS* m_ecs;
    std::map<std::pair<Entity, Entity>, bool> m_previousCollisions; // Track for enter/exit events
    int m_collisionCount;
    int m_checksPerformed;
};

#endif // OMEGA_COLLISION_H
