#ifndef OMEGA_ECS_H
#define OMEGA_ECS_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "Sprite.h"

// Entity is just an ID
using Entity = unsigned int;

// Base component class
struct Component {
    virtual ~Component() = default;
};

// Transform component
struct Transform : public Component {
    Vector2 position;
    Vector2 scale;
    float rotation;
    
    Transform() : position(0, 0), scale(1, 1), rotation(0) {}
};

// Sprite component
struct SpriteComponent : public Component {
    Sprite sprite;
    bool visible;
    
    SpriteComponent() : visible(true) {}
};

// Simple ECS Manager
class ECS {
public:
    ECS();
    ~ECS();

    Entity createEntity();
    void destroyEntity(Entity entity);

    template<typename T>
    T* addComponent(Entity entity);

    template<typename T>
    T* getComponent(Entity entity);

    template<typename T>
    bool hasComponent(Entity entity);

    template<typename T>
    void removeComponent(Entity entity);

    std::vector<Entity> getEntities() const { return m_entities; }

private:
    unsigned int m_nextEntityID;
    std::vector<Entity> m_entities;
    std::unordered_map<Entity, std::unordered_map<std::type_index, std::shared_ptr<Component>>> m_components;
};

// Template implementations
template<typename T>
T* ECS::addComponent(Entity entity) {
    auto component = std::make_shared<T>();
    m_components[entity][std::type_index(typeid(T))] = component;
    return component.get();
}

template<typename T>
T* ECS::getComponent(Entity entity) {
    auto entityIt = m_components.find(entity);
    if (entityIt == m_components.end()) return nullptr;

    auto compIt = entityIt->second.find(std::type_index(typeid(T)));
    if (compIt == entityIt->second.end()) return nullptr;

    return static_cast<T*>(compIt->second.get());
}

template<typename T>
bool ECS::hasComponent(Entity entity) {
    return getComponent<T>(entity) != nullptr;
}

template<typename T>
void ECS::removeComponent(Entity entity) {
    auto entityIt = m_components.find(entity);
    if (entityIt != m_components.end()) {
        entityIt->second.erase(std::type_index(typeid(T)));
    }
}

#endif // OMEGA_ECS_H
