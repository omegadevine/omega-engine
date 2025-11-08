#ifndef OMEGA_EXAMPLE_SCENES_H
#define OMEGA_EXAMPLE_SCENES_H

#include "Scene.h"
#include "AnimatedSprite.h"
#include <vector>

// Main Menu Scene
class MenuScene : public Scene {
public:
    MenuScene();
    ~MenuScene() override = default;
    
    void onEnter() override;
    void onExit() override;
    
    void handleInput(Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer) override;

private:
    int m_selectedOption;
    float m_time;
};

// Gameplay Scene
class GameScene : public Scene {
public:
    GameScene();
    ~GameScene() override = default;
    
    void onEnter() override;
    void onExit() override;
    void onPause() override;
    void onResume() override;
    
    void handleInput(Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer) override;

private:
    Entity m_player;
    std::vector<Entity> m_obstacles;
    AnimatedSprite m_playerAnimSprite;
    float m_time;
    bool m_isMoving;
};

// Pause Scene
class PauseScene : public Scene {
public:
    PauseScene();
    ~PauseScene() override = default;
    
    void onEnter() override;
    void onExit() override;
    
    void handleInput(Input& input) override;
    void update(float deltaTime) override;
    void render(Renderer& renderer) override;

private:
    int m_selectedOption;
};

#endif // OMEGA_EXAMPLE_SCENES_H
