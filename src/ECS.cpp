#include "ECS.h"

ECS::ECS() : m_nextEntityID(1) {
}

ECS::~ECS() {
    m_components.clear();
    m_entities.clear();
}

Entity ECS::createEntity() {
    Entity entity = m_nextEntityID++;
    m_entities.push_back(entity);
    return entity;
}

void ECS::destroyEntity(Entity entity) {
    // Remove from entities list
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }

    // Remove all components
    m_components.erase(entity);
}
