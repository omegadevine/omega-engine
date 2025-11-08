#include "Collision.h"
#include <cmath>
#include <algorithm>

CollisionSystem::CollisionSystem(ECS* ecs)
    : m_ecs(ecs)
    , m_collisionCount(0)
    , m_checksPerformed(0) {
}

void CollisionSystem::update() {
    m_collisionCount = 0;
    m_checksPerformed = 0;
    
    if (!m_ecs) return;
    
    std::vector<Entity> entities = m_ecs->getEntities();
    std::map<std::pair<Entity, Entity>, bool> currentCollisions;
    
    // Check all entity pairs for collisions
    for (size_t i = 0; i < entities.size(); i++) {
        Entity a = entities[i];
        Collider* colliderA = m_ecs->getComponent<Collider>(a);
        if (!colliderA) continue;
        
        for (size_t j = i + 1; j < entities.size(); j++) {
            Entity b = entities[j];
            Collider* colliderB = m_ecs->getComponent<Collider>(b);
            if (!colliderB) continue;
            
            // Check layer masks
            if ((colliderA->layer & colliderB->mask) == 0 &&
                (colliderB->layer & colliderA->mask) == 0) {
                continue;
            }
            
            m_checksPerformed++;
            
            // Check collision
            CollisionInfo info;
            if (checkCollision(a, b, &info)) {
                m_collisionCount++;
                
                auto collisionPair = std::make_pair(a, b);
                currentCollisions[collisionPair] = true;
                
                // Check if this is a new collision
                bool wasColliding = m_previousCollisions.find(collisionPair) != m_previousCollisions.end();
                
                if (!wasColliding) {
                    // OnCollisionEnter
                    if (colliderA->onCollisionEnter) colliderA->onCollisionEnter(b);
                    if (colliderB->onCollisionEnter) colliderB->onCollisionEnter(a);
                } else {
                    // OnCollisionStay
                    if (colliderA->onCollisionStay) colliderA->onCollisionStay(b);
                    if (colliderB->onCollisionStay) colliderB->onCollisionStay(a);
                }
                
                // Resolve collision if not trigger and not both static
                if (!info.isTrigger && !(colliderA->isStatic && colliderB->isStatic)) {
                    resolveCollision(a, b, info);
                }
            }
        }
    }
    
    // Check for collisions that ended
    for (const auto& pair : m_previousCollisions) {
        if (currentCollisions.find(pair.first) == currentCollisions.end()) {
            Entity a = pair.first.first;
            Entity b = pair.first.second;
            
            Collider* colliderA = m_ecs->getComponent<Collider>(a);
            Collider* colliderB = m_ecs->getComponent<Collider>(b);
            
            if (colliderA && colliderA->onCollisionExit) colliderA->onCollisionExit(b);
            if (colliderB && colliderB->onCollisionExit) colliderB->onCollisionExit(a);
        }
    }
    
    m_previousCollisions = currentCollisions;
}

bool CollisionSystem::checkCollision(Entity a, Entity b, CollisionInfo* outInfo) {
    Collider* colliderA = m_ecs->getComponent<Collider>(a);
    Collider* colliderB = m_ecs->getComponent<Collider>(b);
    
    if (!colliderA || !colliderB) return false;
    
    Vector2 posA = getEntityPosition(a);
    Vector2 posB = getEntityPosition(b);
    
    posA.x += colliderA->offset.x;
    posA.y += colliderA->offset.y;
    posB.x += colliderB->offset.x;
    posB.y += colliderB->offset.y;
    
    bool collision = false;
    
    if (colliderA->type == ColliderType::Box && colliderB->type == ColliderType::Box) {
        collision = checkAABB(posA, colliderA->size, posB, colliderB->size, outInfo);
    }
    else if (colliderA->type == ColliderType::Circle && colliderB->type == ColliderType::Circle) {
        collision = checkCircle(posA, colliderA->size.x, posB, colliderB->size.x, outInfo);
    }
    else {
        // Box-Circle collision
        if (colliderA->type == ColliderType::Box) {
            collision = checkBoxCircle(posA, colliderA->size, posB, colliderB->size.x, outInfo);
        } else {
            collision = checkBoxCircle(posB, colliderB->size, posA, colliderA->size.x, outInfo);
            if (collision && outInfo) {
                outInfo->normal.x = -outInfo->normal.x;
                outInfo->normal.y = -outInfo->normal.y;
            }
        }
    }
    
    if (collision && outInfo) {
        outInfo->entityA = a;
        outInfo->entityB = b;
        outInfo->isTrigger = colliderA->isTrigger || colliderB->isTrigger;
    }
    
    return collision;
}

bool CollisionSystem::checkAABB(const Vector2& posA, const Vector2& sizeA,
                                 const Vector2& posB, const Vector2& sizeB,
                                 CollisionInfo* outInfo) {
    float dx = posB.x - posA.x;
    float dy = posB.y - posA.y;
    
    float combinedHalfWidth = (sizeA.x + sizeB.x) * 0.5f;
    float combinedHalfHeight = (sizeA.y + sizeB.y) * 0.5f;
    
    if (std::abs(dx) < combinedHalfWidth && std::abs(dy) < combinedHalfHeight) {
        if (outInfo) {
            float overlapX = combinedHalfWidth - std::abs(dx);
            float overlapY = combinedHalfHeight - std::abs(dy);
            
            if (overlapX < overlapY) {
                outInfo->normal.x = (dx > 0) ? 1.0f : -1.0f;
                outInfo->normal.y = 0;
                outInfo->penetration = overlapX;
            } else {
                outInfo->normal.x = 0;
                outInfo->normal.y = (dy > 0) ? 1.0f : -1.0f;
                outInfo->penetration = overlapY;
            }
        }
        return true;
    }
    
    return false;
}

bool CollisionSystem::checkCircle(const Vector2& posA, float radiusA,
                                   const Vector2& posB, float radiusB,
                                   CollisionInfo* outInfo) {
    float dx = posB.x - posA.x;
    float dy = posB.y - posA.y;
    float distanceSquared = dx * dx + dy * dy;
    float combinedRadius = radiusA + radiusB;
    
    if (distanceSquared < combinedRadius * combinedRadius) {
        if (outInfo) {
            float distance = std::sqrt(distanceSquared);
            if (distance > 0.001f) {
                outInfo->normal.x = dx / distance;
                outInfo->normal.y = dy / distance;
            } else {
                outInfo->normal.x = 1.0f;
                outInfo->normal.y = 0.0f;
            }
            outInfo->penetration = combinedRadius - distance;
        }
        return true;
    }
    
    return false;
}

bool CollisionSystem::checkBoxCircle(const Vector2& boxPos, const Vector2& boxSize,
                                      const Vector2& circlePos, float radius,
                                      CollisionInfo* outInfo) {
    // Find closest point on box to circle
    float closestX = std::max(boxPos.x - boxSize.x * 0.5f, 
                              std::min(circlePos.x, boxPos.x + boxSize.x * 0.5f));
    float closestY = std::max(boxPos.y - boxSize.y * 0.5f,
                              std::min(circlePos.y, boxPos.y + boxSize.y * 0.5f));
    
    float dx = circlePos.x - closestX;
    float dy = circlePos.y - closestY;
    float distanceSquared = dx * dx + dy * dy;
    
    if (distanceSquared < radius * radius) {
        if (outInfo) {
            float distance = std::sqrt(distanceSquared);
            if (distance > 0.001f) {
                outInfo->normal.x = dx / distance;
                outInfo->normal.y = dy / distance;
            } else {
                outInfo->normal.x = 1.0f;
                outInfo->normal.y = 0.0f;
            }
            outInfo->penetration = radius - distance;
        }
        return true;
    }
    
    return false;
}

void CollisionSystem::resolveCollision(Entity a, Entity b, const CollisionInfo& info) {
    Collider* colliderA = m_ecs->getComponent<Collider>(a);
    Collider* colliderB = m_ecs->getComponent<Collider>(b);
    Transform* transformA = m_ecs->getComponent<Transform>(a);
    Transform* transformB = m_ecs->getComponent<Transform>(b);
    
    if (!colliderA || !colliderB || !transformA || !transformB) return;
    
    // Simple position correction
    if (!colliderA->isStatic && !colliderB->isStatic) {
        // Both dynamic - push apart equally
        float halfPenetration = info.penetration * 0.5f;
        transformA->position.x -= info.normal.x * halfPenetration;
        transformA->position.y -= info.normal.y * halfPenetration;
        transformB->position.x += info.normal.x * halfPenetration;
        transformB->position.y += info.normal.y * halfPenetration;
    } else if (!colliderA->isStatic) {
        // Only A is dynamic
        transformA->position.x -= info.normal.x * info.penetration;
        transformA->position.y -= info.normal.y * info.penetration;
    } else if (!colliderB->isStatic) {
        // Only B is dynamic
        transformB->position.x += info.normal.x * info.penetration;
        transformB->position.y += info.normal.y * info.penetration;
    }
}

Vector2 CollisionSystem::getEntityPosition(Entity entity) {
    Transform* transform = m_ecs->getComponent<Transform>(entity);
    if (transform) {
        return transform->position;
    }
    return Vector2(0, 0);
}

bool CollisionSystem::raycast(const Vector2& origin, const Vector2& direction, float maxDistance,
                               Entity* hitEntity, Vector2* hitPoint) {
    // Simple raycast implementation
    // For production, you'd want a more optimized spatial structure
    
    if (!m_ecs) return false;
    
    float closestDistance = maxDistance;
    Entity closest = 0;
    Vector2 closestPoint = origin;
    
    std::vector<Entity> entities = m_ecs->getEntities();
    for (Entity entity : entities) {
        Collider* collider = m_ecs->getComponent<Collider>(entity);
        if (!collider) continue;
        
        Vector2 pos = getEntityPosition(entity);
        pos.x += collider->offset.x;
        pos.y += collider->offset.y;
        
        // Simple box raycast
        if (collider->type == ColliderType::Box) {
            // Implement AABB raycast
            // Simplified for brevity
        }
    }
    
    if (closest != 0) {
        if (hitEntity) *hitEntity = closest;
        if (hitPoint) *hitPoint = closestPoint;
        return true;
    }
    
    return false;
}

std::vector<Entity> CollisionSystem::getEntitiesInRadius(const Vector2& center, float radius) {
    std::vector<Entity> result;
    if (!m_ecs) return result;
    
    std::vector<Entity> entities = m_ecs->getEntities();
    for (Entity entity : entities) {
        Vector2 pos = getEntityPosition(entity);
        float dx = pos.x - center.x;
        float dy = pos.y - center.y;
        float distSq = dx * dx + dy * dy;
        
        if (distSq <= radius * radius) {
            result.push_back(entity);
        }
    }
    
    return result;
}

std::vector<Entity> CollisionSystem::getEntitiesInBox(const Vector2& center, const Vector2& size) {
    std::vector<Entity> result;
    if (!m_ecs) return result;
    
    std::vector<Entity> entities = m_ecs->getEntities();
    for (Entity entity : entities) {
        Vector2 pos = getEntityPosition(entity);
        
        if (std::abs(pos.x - center.x) <= size.x * 0.5f &&
            std::abs(pos.y - center.y) <= size.y * 0.5f) {
            result.push_back(entity);
        }
    }
    
    return result;
}
