#ifndef OMEGA_SCENE_MANAGER_H
#define OMEGA_SCENE_MANAGER_H

#include "Scene.h"
#include <vector>
#include <map>
#include <memory>
#include <functional>

class SceneManager {
public:
    SceneManager();
    ~SceneManager() = default;
    
    // Scene registration
    void registerScene(const std::string& name, std::function<std::unique_ptr<Scene>()> factory);
    
    // Scene transitions
    void changeScene(const std::string& name);    // Replace current scene
    void pushScene(const std::string& name);      // Push new scene on stack
    void popScene();                               // Pop current scene, resume previous
    
    // Scene stack management
    Scene* getCurrentScene();
    size_t getSceneStackSize() const { return m_sceneStack.size(); }
    
    // Update/render current scene
    void handleInput(Input& input);
    void update(float deltaTime);
    void render(Renderer& renderer);
    
    // Utility
    bool hasScene(const std::string& name) const;
    void clearSceneStack();

private:
    void activateScene(Scene* scene);
    void deactivateScene(Scene* scene);
    
    std::map<std::string, std::function<std::unique_ptr<Scene>()>> m_sceneFactories;
    std::vector<std::unique_ptr<Scene>> m_sceneStack;
};

#endif // OMEGA_SCENE_MANAGER_H
