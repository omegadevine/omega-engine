#ifndef OMEGA_SCENE_H
#define OMEGA_SCENE_H

#include "ECS.h"
#include "Collision.h"
#include "Camera.h"
#include "Input.h"
#include "Renderer.h"
#include "AssetManager.h"
#include <string>
#include <memory>

// Forward declarations
class SceneManager;

// Base scene class
class Scene {
public:
    Scene(const std::string& name);
    virtual ~Scene() = default;
    
    // Scene lifecycle
    virtual void onEnter() {}      // Called when scene becomes active
    virtual void onExit() {}       // Called when scene is deactivated
    virtual void onPause() {}      // Called when another scene is pushed on top
    virtual void onResume() {}     // Called when scene becomes active again
    
    // Core loop
    virtual void handleInput(Input& input) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(Renderer& renderer) = 0;
    
    // Getters
    std::string getName() const { return m_name; }
    bool isActive() const { return m_active; }
    bool isPaused() const { return m_paused; }
    
    // Scene manager access
    void setSceneManager(SceneManager* manager) { m_sceneManager = manager; }
    
protected:
    std::string m_name;
    bool m_active;
    bool m_paused;
    SceneManager* m_sceneManager;
    
    // Each scene has its own ECS
    std::unique_ptr<ECS> m_ecs;
    std::unique_ptr<CollisionSystem> m_collisionSystem;
    std::unique_ptr<Camera> m_camera;
};

#endif // OMEGA_SCENE_H
