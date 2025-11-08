#ifndef OMEGA_PHYSICS_H
#define OMEGA_PHYSICS_H

#include "Sprite.h"
#include "ECS.h"
#include <vector>
#include <memory>
#include <functional>

// Forward declarations for Box2D (would include box2d/box2d.h in real implementation)
class b2World;
class b2Body;
class b2Fixture;
struct b2Vec2;

// Physics body types
enum class BodyType {
    Static,       // Doesn't move (walls, ground)
    Dynamic,      // Moves and collides (player, enemies)
    Kinematic     // Moves but ignores physics (platforms)
};

// Physics shape types
enum class ShapeType {
    Box,
    Circle,
    Polygon
};

// Physics body definition
struct PhysicsBodyDef {
    BodyType type = BodyType::Dynamic;
    Vector2 position;
    float rotation = 0.0f;
    Vector2 linearVelocity;
    float angularVelocity = 0.0f;
    float linearDamping = 0.0f;
    float angularDamping = 0.0f;
    float gravityScale = 1.0f;
    bool fixedRotation = false;
    bool isBullet = false;
    bool enabled = true;
};

// Physics shape definition
struct PhysicsShapeDef {
    ShapeType type = ShapeType::Box;
    Vector2 size;           // For box
    float radius = 0.5f;    // For circle
    std::vector<Vector2> vertices; // For polygon
    
    float density = 1.0f;
    float friction = 0.3f;
    float restitution = 0.0f; // Bounciness
    bool isSensor = false;    // Trigger volume
    uint16_t categoryBits = 0x0001;
    uint16_t maskBits = 0xFFFF;
};

// Physics body wrapper
class PhysicsBody {
public:
    PhysicsBody(b2Body* body);
    ~PhysicsBody() = default;
    
    // Transform
    void setPosition(const Vector2& pos);
    Vector2 getPosition() const;
    void setRotation(float angle);
    float getRotation() const;
    
    // Velocity
    void setLinearVelocity(const Vector2& vel);
    Vector2 getLinearVelocity() const;
    void setAngularVelocity(float vel);
    float getAngularVelocity() const;
    
    // Forces and impulses
    void applyForce(const Vector2& force, const Vector2& point);
    void applyForceToCenter(const Vector2& force);
    void applyLinearImpulse(const Vector2& impulse, const Vector2& point);
    void applyLinearImpulseToCenter(const Vector2& impulse);
    void applyAngularImpulse(float impulse);
    void applyTorque(float torque);
    
    // Properties
    void setMass(float mass);
    float getMass() const;
    void setGravityScale(float scale);
    float getGravityScale() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setFixedRotation(bool fixed);
    bool isFixedRotation() const;
    
    // User data
    void setUserData(void* data);
    void* getUserData() const;
    
    b2Body* getB2Body() const { return m_body; }

private:
    b2Body* m_body;
};

// Collision callback interface
class ICollisionListener {
public:
    virtual ~ICollisionListener() = default;
    virtual void onCollisionBegin(PhysicsBody* bodyA, PhysicsBody* bodyB) {}
    virtual void onCollisionEnd(PhysicsBody* bodyA, PhysicsBody* bodyB) {}
    virtual void onSensorBegin(PhysicsBody* body, PhysicsBody* sensor) {}
    virtual void onSensorEnd(PhysicsBody* body, PhysicsBody* sensor) {}
};

// Raycast result
struct RaycastHit {
    PhysicsBody* body = nullptr;
    Vector2 point;
    Vector2 normal;
    float fraction = 0.0f;
};

// Physics world
class PhysicsWorld {
public:
    PhysicsWorld(const Vector2& gravity = Vector2(0, -10.0f));
    ~PhysicsWorld();
    
    // Simulation
    void step(float deltaTime);
    void setGravity(const Vector2& gravity);
    Vector2 getGravity() const;
    
    // Body creation
    PhysicsBody* createBody(const PhysicsBodyDef& bodyDef);
    void destroyBody(PhysicsBody* body);
    
    // Shape attachment
    void addShape(PhysicsBody* body, const PhysicsShapeDef& shapeDef);
    
    // Collision
    void setCollisionListener(ICollisionListener* listener);
    
    // Queries
    std::vector<PhysicsBody*> queryAABB(const Vector2& lowerBound, const Vector2& upperBound);
    bool raycast(const Vector2& start, const Vector2& end, RaycastHit& hit);
    std::vector<RaycastHit> raycastAll(const Vector2& start, const Vector2& end);
    
    // Settings
    void setVelocityIterations(int iterations) { m_velocityIterations = iterations; }
    void setPositionIterations(int iterations) { m_positionIterations = iterations; }
    void setTimeStep(float timeStep) { m_timeStep = timeStep; }
    
    // Debug
    void debugDraw(class DebugRenderer* debugRenderer);

private:
    b2World* m_world;
    ICollisionListener* m_collisionListener;
    std::vector<std::unique_ptr<PhysicsBody>> m_bodies;
    int m_velocityIterations;
    int m_positionIterations;
    float m_timeStep;
    float m_accumulator;
};

// Physics component for ECS
struct PhysicsComponent {
    PhysicsBody* body = nullptr;
    bool syncTransform = true;
};

// Physics system for ECS
class PhysicsSystem {
public:
    PhysicsSystem(PhysicsWorld* world);
    ~PhysicsSystem() = default;
    
    void update(ECS& ecs, float deltaTime);
    
    // Helper to create physics-enabled entity
    Entity createPhysicsEntity(ECS& ecs, const PhysicsBodyDef& bodyDef, const PhysicsShapeDef& shapeDef);

private:
    PhysicsWorld* m_world;
};

#endif // OMEGA_PHYSICS_H
