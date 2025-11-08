#include "SceneManager.h"
#include <iostream>

SceneManager::SceneManager() {
}

void SceneManager::registerScene(const std::string& name, std::function<std::unique_ptr<Scene>()> factory) {
    m_sceneFactories[name] = factory;
    std::cout << "SceneManager: Registered scene '" << name << "'" << std::endl;
}

void SceneManager::changeScene(const std::string& name) {
    auto it = m_sceneFactories.find(name);
    if (it == m_sceneFactories.end()) {
        std::cerr << "SceneManager: Scene '" << name << "' not found" << std::endl;
        return;
    }
    
    // Deactivate and remove all scenes
    while (!m_sceneStack.empty()) {
        deactivateScene(m_sceneStack.back().get());
        m_sceneStack.pop_back();
    }
    
    // Create and activate new scene
    std::unique_ptr<Scene> newScene = it->second();
    newScene->setSceneManager(this);
    activateScene(newScene.get());
    m_sceneStack.push_back(std::move(newScene));
    
    std::cout << "SceneManager: Changed to scene '" << name << "'" << std::endl;
}

void SceneManager::pushScene(const std::string& name) {
    auto it = m_sceneFactories.find(name);
    if (it == m_sceneFactories.end()) {
        std::cerr << "SceneManager: Scene '" << name << "' not found" << std::endl;
        return;
    }
    
    // Pause current scene
    if (!m_sceneStack.empty()) {
        m_sceneStack.back()->onPause();
    }
    
    // Create and activate new scene
    std::unique_ptr<Scene> newScene = it->second();
    newScene->setSceneManager(this);
    activateScene(newScene.get());
    m_sceneStack.push_back(std::move(newScene));
    
    std::cout << "SceneManager: Pushed scene '" << name << "' (stack size: " << m_sceneStack.size() << ")" << std::endl;
}

void SceneManager::popScene() {
    if (m_sceneStack.empty()) {
        std::cerr << "SceneManager: Cannot pop, scene stack is empty" << std::endl;
        return;
    }
    
    // Deactivate and remove current scene
    deactivateScene(m_sceneStack.back().get());
    std::string poppedName = m_sceneStack.back()->getName();
    m_sceneStack.pop_back();
    
    // Resume previous scene if it exists
    if (!m_sceneStack.empty()) {
        m_sceneStack.back()->onResume();
    }
    
    std::cout << "SceneManager: Popped scene '" << poppedName << "' (stack size: " << m_sceneStack.size() << ")" << std::endl;
}

Scene* SceneManager::getCurrentScene() {
    if (m_sceneStack.empty()) {
        return nullptr;
    }
    return m_sceneStack.back().get();
}

void SceneManager::handleInput(Input& input) {
    Scene* current = getCurrentScene();
    if (current && current->isActive() && !current->isPaused()) {
        current->handleInput(input);
    }
}

void SceneManager::update(float deltaTime) {
    Scene* current = getCurrentScene();
    if (current && current->isActive() && !current->isPaused()) {
        current->update(deltaTime);
    }
}

void SceneManager::render(Renderer& renderer) {
    Scene* current = getCurrentScene();
    if (current && current->isActive()) {
        current->render(renderer);
    }
}

bool SceneManager::hasScene(const std::string& name) const {
    return m_sceneFactories.find(name) != m_sceneFactories.end();
}

void SceneManager::clearSceneStack() {
    while (!m_sceneStack.empty()) {
        deactivateScene(m_sceneStack.back().get());
        m_sceneStack.pop_back();
    }
    std::cout << "SceneManager: Cleared scene stack" << std::endl;
}

void SceneManager::activateScene(Scene* scene) {
    if (!scene) return;
    scene->m_active = true;
    scene->m_paused = false;
    scene->onEnter();
}

void SceneManager::deactivateScene(Scene* scene) {
    if (!scene) return;
    scene->onExit();
    scene->m_active = false;
}
