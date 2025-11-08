#include "Scene.h"

Scene::Scene(const std::string& name)
    : m_name(name)
    , m_active(false)
    , m_paused(false)
    , m_sceneManager(nullptr)
    , m_ecs(std::make_unique<ECS>())
    , m_collisionSystem(nullptr)
    , m_camera(nullptr) {
}
