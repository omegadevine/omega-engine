#include "Physics.h"
#include "Debug.h"
#include <iostream>
#include <cmath>

// NOTE: This is a stub implementation. In a real project, you would link against Box2D
// and implement these methods using the actual Box2D API.
// For now, we provide the interface and basic simulation without Box2D dependency.

// ============================================================================
// Simple Physics Simulation (Box2D Stub)
// ============================================================================

struct SimpleBody {
    BodyType type;
    Vector2 position;
    float rotation;
    Vector2 velocity;
    float angularVelocity;
    float mass;
    float gravityScale;
    bool fixedRotation;
    bool enabled;
    void* userData;
    std::vector<PhysicsShapeDef> shapes;
};

// Temporary world implementation without Box2D
struct SimpleWorld {
    Vector2 gravity;
    std::vector<SimpleBody> bodies;
};

// ============================================================================
// PhysicsBody Implementation
// ============================================================================

PhysicsBody::PhysicsBody(b2Body* body)
    : m_body(body) {
}

void PhysicsBody::setPosition(const Vector2& pos) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->position = pos;
}

Vector2 PhysicsBody::getPosition() const {
    if (!m_body) return Vector2(0, 0);
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->position;
}

void PhysicsBody::setRotation(float angle) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->rotation = angle;
}

float PhysicsBody::getRotation() const {
    if (!m_body) return 0.0f;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->rotation;
}

void PhysicsBody::setLinearVelocity(const Vector2& vel) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->velocity = vel;
}

Vector2 PhysicsBody::getLinearVelocity() const {
    if (!m_body) return Vector2(0, 0);
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->velocity;
}

void PhysicsBody::setAngularVelocity(float vel) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->angularVelocity = vel;
}

float PhysicsBody::getAngularVelocity() const {
    if (!m_body) return 0.0f;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->angularVelocity;
}

void PhysicsBody::applyForce(const Vector2& force, const Vector2& point) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    if (body->type != BodyType::Dynamic) return;
    
    // F = ma, so a = F/m
    Vector2 acceleration(force.x / body->mass, force.y / body->mass);
    body->velocity.x += acceleration.x;
    body->velocity.y += acceleration.y;
}

void PhysicsBody::applyForceToCenter(const Vector2& force) {
    applyForce(force, getPosition());
}

void PhysicsBody::applyLinearImpulse(const Vector2& impulse, const Vector2& point) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    if (body->type != BodyType::Dynamic) return;
    
    // Impulse directly changes velocity
    body->velocity.x += impulse.x / body->mass;
    body->velocity.y += impulse.y / body->mass;
}

void PhysicsBody::applyLinearImpulseToCenter(const Vector2& impulse) {
    applyLinearImpulse(impulse, getPosition());
}

void PhysicsBody::applyAngularImpulse(float impulse) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    if (body->type != BodyType::Dynamic || body->fixedRotation) return;
    
    body->angularVelocity += impulse;
}

void PhysicsBody::applyTorque(float torque) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    if (body->type != BodyType::Dynamic || body->fixedRotation) return;
    
    body->angularVelocity += torque / body->mass;
}

void PhysicsBody::setMass(float mass) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->mass = mass;
}

float PhysicsBody::getMass() const {
    if (!m_body) return 1.0f;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->mass;
}

void PhysicsBody::setGravityScale(float scale) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->gravityScale = scale;
}

float PhysicsBody::getGravityScale() const {
    if (!m_body) return 1.0f;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->gravityScale;
}

void PhysicsBody::setEnabled(bool enabled) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->enabled = enabled;
}

bool PhysicsBody::isEnabled() const {
    if (!m_body) return false;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->enabled;
}

void PhysicsBody::setFixedRotation(bool fixed) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->fixedRotation = fixed;
}

bool PhysicsBody::isFixedRotation() const {
    if (!m_body) return false;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->fixedRotation;
}

void PhysicsBody::setUserData(void* data) {
    if (!m_body) return;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    body->userData = data;
}

void* PhysicsBody::getUserData() const {
    if (!m_body) return nullptr;
    SimpleBody* body = reinterpret_cast<SimpleBody*>(m_body);
    return body->userData;
}

// ============================================================================
// PhysicsWorld Implementation
// ============================================================================

PhysicsWorld::PhysicsWorld(const Vector2& gravity)
    : m_world(nullptr)
    , m_collisionListener(nullptr)
    , m_velocityIterations(8)
    , m_positionIterations(3)
    , m_timeStep(1.0f / 60.0f)
    , m_accumulator(0.0f) {
    
    // Create simple world
    SimpleWorld* world = new SimpleWorld();
    world->gravity = gravity;
    m_world = reinterpret_cast<b2World*>(world);
    
    std::cout << "PhysicsWorld: Created (stub implementation)" << std::endl;
}

PhysicsWorld::~PhysicsWorld() {
    SimpleWorld* world = reinterpret_cast<SimpleWorld*>(m_world);
    delete world;
}

void PhysicsWorld::step(float deltaTime) {
    SimpleWorld* world = reinterpret_cast<SimpleWorld*>(m_world);
    if (!world) return;
    
    m_accumulator += deltaTime;
    
    while (m_accumulator >= m_timeStep) {
        // Simple physics integration
        for (auto& simpleBody : world->bodies) {
            if (!simpleBody.enabled || simpleBody.type != BodyType::Dynamic) continue;
            
            // Apply gravity
            Vector2 gravityForce(
                world->gravity.x * simpleBody.gravityScale,
                world->gravity.y * simpleBody.gravityScale
            );
            simpleBody.velocity.x += gravityForce.x * m_timeStep;
            simpleBody.velocity.y += gravityForce.y * m_timeStep;
            
            // Update position
            simpleBody.position.x += simpleBody.velocity.x * m_timeStep;
            simpleBody.position.y += simpleBody.velocity.y * m_timeStep;
            
            // Update rotation
            if (!simpleBody.fixedRotation) {
                simpleBody.rotation += simpleBody.angularVelocity * m_timeStep;
            }
        }
        
        m_accumulator -= m_timeStep;
    }
}

void PhysicsWorld::setGravity(const Vector2& gravity) {
    SimpleWorld* world = reinterpret_cast<SimpleWorld*>(m_world);
    if (world) {
        world->gravity = gravity;
    }
}

Vector2 PhysicsWorld::getGravity() const {
    SimpleWorld* world = reinterpret_cast<SimpleWorld*>(m_world);
    return world ? world->gravity : Vector2(0, -10);
}

PhysicsBody* PhysicsWorld::createBody(const PhysicsBodyDef& bodyDef) {
    SimpleWorld* world = reinterpret_cast<SimpleWorld*>(m_world);
    if (!world) return nullptr;
    
    SimpleBody simpleBody;
    simpleBody.type = bodyDef.type;
    simpleBody.position = bodyDef.position;
    simpleBody.rotation = bodyDef.rotation;
    simpleBody.velocity = bodyDef.linearVelocity;
    simpleBody.angularVelocity = bodyDef.angularVelocity;
    simpleBody.mass = 1.0f;
    simpleBody.gravityScale = bodyDef.gravityScale;
    simpleBody.fixedRotation = bodyDef.fixedRotation;
    simpleBody.enabled = bodyDef.enabled;
    simpleBody.userData = nullptr;
    
    world->bodies.push_back(simpleBody);
    
    b2Body* b2body = reinterpret_cast<b2Body*>(&world->bodies.back());
    auto body = std::make_unique<PhysicsBody>(b2body);
    PhysicsBody* ptr = body.get();
    m_bodies.push_back(std::move(body));
    
    return ptr;
}

void PhysicsWorld::destroyBody(PhysicsBody* body) {
    m_bodies.erase(
        std::remove_if(m_bodies.begin(), m_bodies.end(),
            [body](const std::unique_ptr<PhysicsBody>& b) {
                return b.get() == body;
            }),
        m_bodies.end()
    );
}

void PhysicsWorld::addShape(PhysicsBody* body, const PhysicsShapeDef& shapeDef) {
    if (!body) return;
    
    SimpleBody* simpleBody = reinterpret_cast<SimpleBody*>(body->getB2Body());
    if (simpleBody) {
        simpleBody->shapes.push_back(shapeDef);
        
        // Calculate mass based on shape
        float area = 1.0f;
        if (shapeDef.type == ShapeType::Box) {
            area = shapeDef.size.x * shapeDef.size.y;
        } else if (shapeDef.type == ShapeType::Circle) {
            area = 3.14159f * shapeDef.radius * shapeDef.radius;
        }
        
        simpleBody->mass += area * shapeDef.density;
    }
}

void PhysicsWorld::setCollisionListener(ICollisionListener* listener) {
    m_collisionListener = listener;
}

std::vector<PhysicsBody*> PhysicsWorld::queryAABB(const Vector2& lowerBound, const Vector2& upperBound) {
    std::vector<PhysicsBody*> results;
    
    // Simple AABB query
    for (auto& body : m_bodies) {
        Vector2 pos = body->getPosition();
        if (pos.x >= lowerBound.x && pos.x <= upperBound.x &&
            pos.y >= lowerBound.y && pos.y <= upperBound.y) {
            results.push_back(body.get());
        }
    }
    
    return results;
}

bool PhysicsWorld::raycast(const Vector2& start, const Vector2& end, RaycastHit& hit) {
    // Simplified raycast - checks against body positions
    float closestFraction = 1.0f;
    PhysicsBody* closestBody = nullptr;
    
    for (auto& body : m_bodies) {
        Vector2 pos = body->getPosition();
        
        // Simple point-distance check
        float dx = pos.x - start.x;
        float dy = pos.y - start.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        float rayLength = std::sqrt((end.x - start.x) * (end.x - start.x) + 
                                   (end.y - start.y) * (end.y - start.y));
        
        float fraction = distance / rayLength;
        
        if (fraction < closestFraction && fraction <= 1.0f) {
            closestFraction = fraction;
            closestBody = body.get();
        }
    }
    
    if (closestBody) {
        hit.body = closestBody;
        hit.fraction = closestFraction;
        hit.point.x = start.x + (end.x - start.x) * closestFraction;
        hit.point.y = start.y + (end.y - start.y) * closestFraction;
        return true;
    }
    
    return false;
}

std::vector<RaycastHit> PhysicsWorld::raycastAll(const Vector2& start, const Vector2& end) {
    std::vector<RaycastHit> hits;
    
    for (auto& body : m_bodies) {
        RaycastHit hit;
        if (raycast(start, end, hit) && hit.body == body.get()) {
            hits.push_back(hit);
        }
    }
    
    return hits;
}

void PhysicsWorld::debugDraw(DebugRenderer* debugRenderer) {
    if (!debugRenderer) return;
    
    SimpleWorld* world = reinterpret_cast<SimpleWorld*>(m_world);
    if (!world) return;
    
    // Draw all bodies
    for (auto& simpleBody : world->bodies) {
        Color color = simpleBody.type == BodyType::Static ? Color(0.5f, 0.5f, 0.5f, 1.0f) :
                      simpleBody.type == BodyType::Dynamic ? Color(0.0f, 1.0f, 0.0f, 1.0f) :
                      Color(1.0f, 1.0f, 0.0f, 1.0f);
        
        // Draw shapes
        for (const auto& shape : simpleBody.shapes) {
            if (shape.type == ShapeType::Box) {
                debugRenderer->drawRect(
                    Vector2(simpleBody.position.x - shape.size.x * 0.5f,
                           simpleBody.position.y - shape.size.y * 0.5f),
                    shape.size,
                    color
                );
            } else if (shape.type == ShapeType::Circle) {
                debugRenderer->drawCircle(simpleBody.position, shape.radius, color);
            }
        }
        
        // Draw velocity vector
        if (simpleBody.type == BodyType::Dynamic) {
            Vector2 end(
                simpleBody.position.x + simpleBody.velocity.x * 0.1f,
                simpleBody.position.y + simpleBody.velocity.y * 0.1f
            );
            debugRenderer->drawLine(simpleBody.position, end, Color(1, 0, 0, 1));
        }
    }
}

// ============================================================================
// PhysicsSystem Implementation
// ============================================================================

PhysicsSystem::PhysicsSystem(PhysicsWorld* world)
    : m_world(world) {
}

void PhysicsSystem::update(ECS& ecs, float deltaTime) {
    if (!m_world) return;
    
    // Step physics
    m_world->step(deltaTime);
    
    // Sync transforms
    for (Entity entity = 0; entity < ecs.getEntityCount(); entity++) {
        if (!ecs.hasComponent<PhysicsComponent>(entity)) continue;
        
        auto* physics = ecs.getComponent<PhysicsComponent>(entity);
        if (!physics || !physics->body || !physics->syncTransform) continue;
        
        // Update entity transform from physics body
        if (ecs.hasComponent<TransformComponent>(entity)) {
            auto* transform = ecs.getComponent<TransformComponent>(entity);
            transform->position = physics->body->getPosition();
            transform->rotation = physics->body->getRotation();
        }
    }
}

Entity PhysicsSystem::createPhysicsEntity(ECS& ecs, const PhysicsBodyDef& bodyDef, const PhysicsShapeDef& shapeDef) {
    Entity entity = ecs.createEntity();
    
    // Add transform
    TransformComponent transform;
    transform.position = bodyDef.position;
    transform.rotation = bodyDef.rotation;
    ecs.addComponent(entity, transform);
    
    // Create physics body
    PhysicsBody* body = m_world->createBody(bodyDef);
    m_world->addShape(body, shapeDef);
    
    // Add physics component
    PhysicsComponent physics;
    physics.body = body;
    physics.syncTransform = true;
    ecs.addComponent(entity, physics);
    
    return entity;
}
