#include "ExampleScenes.h"
#include "SceneManager.h"
#include <iostream>
#include <cmath>

// ============================================================================
// MenuScene Implementation
// ============================================================================

MenuScene::MenuScene() 
    : Scene("Menu")
    , m_selectedOption(0)
    , m_time(0.0f) {
}

void MenuScene::onEnter() {
    std::cout << "MenuScene: Entered" << std::endl;
    m_selectedOption = 0;
    m_time = 0.0f;
}

void MenuScene::onExit() {
    std::cout << "MenuScene: Exited" << std::endl;
}

void MenuScene::handleInput(Input& input) {
    if (input.isKeyJustPressed(KeyCode::Up)) {
        m_selectedOption = (m_selectedOption - 1 + 2) % 2;
    }
    if (input.isKeyJustPressed(KeyCode::Down)) {
        m_selectedOption = (m_selectedOption + 1) % 2;
    }
    if (input.isKeyJustPressed(KeyCode::Enter) || input.isKeyJustPressed(KeyCode::Space)) {
        if (m_selectedOption == 0) {
            m_sceneManager->changeScene("Game");
        } else if (m_selectedOption == 1) {
            // Quit (handled by main loop)
            std::cout << "MenuScene: Quit selected" << std::endl;
        }
    }
}

void MenuScene::update(float deltaTime) {
    m_time += deltaTime;
}

void MenuScene::render(Renderer& renderer) {
    // Animated background color
    float r = 0.1f + 0.05f * std::sin(m_time * 0.5f);
    float g = 0.1f + 0.05f * std::sin(m_time * 0.7f);
    float b = 0.2f + 0.05f * std::sin(m_time * 0.3f);
    renderer.clear(r, g, b, 1.0f);
    
    // In a real game, you'd render text/sprites here
    std::cout << "\r[MENU] " << (m_selectedOption == 0 ? "> Start Game" : "  Start Game")
              << " | " << (m_selectedOption == 1 ? "> Quit" : "  Quit") << "          " << std::flush;
}

// ============================================================================
// GameScene Implementation
// ============================================================================

GameScene::GameScene()
    : Scene("Game")
    , m_player(0)
    , m_time(0.0f)
    , m_isMoving(false) {
    
    // Initialize camera
    m_camera = std::make_unique<Camera>(800.0f, 600.0f);
    m_camera->setFollowSpeed(3.0f);
    m_camera->setBounds(0, 0, 1600, 1200);
}

void GameScene::onEnter() {
    std::cout << "GameScene: Entered" << std::endl;
    
    // Initialize collision system
    m_collisionSystem = std::make_unique<CollisionSystem>(m_ecs.get());
    
    // Get texture from asset manager
    AssetManager& assets = AssetManager::getInstance();
    Texture* testTexture = assets.getTexture("test_pattern");
    
    // Create player
    m_player = m_ecs->createEntity();
    auto* playerTransform = m_ecs->addComponent<Transform>(m_player);
    playerTransform->position = Vector2(400.0f, 300.0f);
    
    auto* playerSprite = m_ecs->addComponent<SpriteComponent>(m_player);
    playerSprite->sprite.setTexture(testTexture);
    playerSprite->sprite.setSize(Vector2(64, 64));
    playerSprite->sprite.setColor(Color(0.3f, 0.7f, 1.0f, 1.0f));
    
    auto* playerCollider = m_ecs->addComponent<Collider>(m_player);
    playerCollider->type = ColliderType::Box;
    playerCollider->size = Vector2(64, 64);
    playerCollider->layer = 1;
    
    // Setup animated sprite
    m_playerAnimSprite.setTexture(testTexture);
    m_playerAnimSprite.setSize(Vector2(64, 64));
    m_playerAnimSprite.setColor(Color(0.3f, 0.7f, 1.0f, 1.0f));
    
    Animation idleAnim("idle");
    idleAnim.setFrameDuration(0.2f);
    idleAnim.setLoop(true);
    for (int i = 0; i < 4; i++) {
        idleAnim.addFrame(i * 64, 0, 64, 64);
    }
    m_playerAnimSprite.addAnimation("idle", idleAnim);
    m_playerAnimSprite.play("idle");
    
    // Create obstacles
    for (int i = 0; i < 5; i++) {
        Entity obstacle = m_ecs->createEntity();
        auto* transform = m_ecs->addComponent<Transform>(obstacle);
        transform->position = Vector2(200.0f + i * 200.0f, 400.0f);
        
        auto* sprite = m_ecs->addComponent<SpriteComponent>(obstacle);
        sprite->sprite.setTexture(testTexture);
        sprite->sprite.setSize(Vector2(48, 48));
        sprite->sprite.setColor(Color(1.0f, 0.5f, 0.2f, 1.0f));
        
        auto* collider = m_ecs->addComponent<Collider>(obstacle);
        collider->type = (i % 2 == 0) ? ColliderType::Box : ColliderType::Circle;
        collider->size = (i % 2 == 0) ? Vector2(48, 48) : Vector2(24, 24);
        collider->layer = 2;
        collider->isStatic = true;
        
        m_obstacles.push_back(obstacle);
    }
}

void GameScene::onExit() {
    std::cout << "GameScene: Exited" << std::endl;
}

void GameScene::onPause() {
    std::cout << "GameScene: Paused" << std::endl;
}

void GameScene::onResume() {
    std::cout << "GameScene: Resumed" << std::endl;
}

void GameScene::handleInput(Input& input) {
    if (input.isKeyJustPressed(KeyCode::Escape)) {
        m_sceneManager->pushScene("Pause");
        return;
    }
    
    auto* playerTransform = m_ecs->getComponent<Transform>(m_player);
    if (!playerTransform) return;
    
    m_isMoving = false;
    const float moveSpeed = 3.0f;
    
    if (input.isKeyPressed(KeyCode::W) || input.isKeyPressed(KeyCode::Up)) {
        playerTransform->position.y -= moveSpeed;
        m_isMoving = true;
    }
    if (input.isKeyPressed(KeyCode::S) || input.isKeyPressed(KeyCode::Down)) {
        playerTransform->position.y += moveSpeed;
        m_isMoving = true;
    }
    if (input.isKeyPressed(KeyCode::A) || input.isKeyPressed(KeyCode::Left)) {
        playerTransform->position.x -= moveSpeed;
        m_isMoving = true;
    }
    if (input.isKeyPressed(KeyCode::D) || input.isKeyPressed(KeyCode::Right)) {
        playerTransform->position.x += moveSpeed;
        m_isMoving = true;
    }
    
    // Keep in bounds
    playerTransform->position.x = std::max(0.0f, std::min(1600.0f - 64.0f, playerTransform->position.x));
    playerTransform->position.y = std::max(0.0f, std::min(1200.0f - 64.0f, playerTransform->position.y));
}

void GameScene::update(float deltaTime) {
    m_time += deltaTime;
    
    auto* playerTransform = m_ecs->getComponent<Transform>(m_player);
    if (playerTransform) {
        m_camera->follow(Vector2(
            playerTransform->position.x + 32,
            playerTransform->position.y + 32
        ));
        m_camera->update(deltaTime);
        
        m_playerAnimSprite.setPosition(playerTransform->position);
        m_playerAnimSprite.update(deltaTime);
    }
    
    m_collisionSystem->update();
}

void GameScene::render(Renderer& renderer) {
    renderer.clear(0.1f, 0.1f, 0.15f, 1.0f);
    
    Shader* shader = AssetManager::getInstance().getShader("sprite_shader");
    if (!shader) return;
    
    // Render player
    m_playerAnimSprite.drawWithCamera(shader, m_camera.get(), 800, 600);
    
    // Render obstacles
    for (Entity entity : m_ecs->getEntities()) {
        if (entity == m_player) continue;
        
        auto* transform = m_ecs->getComponent<Transform>(entity);
        auto* spriteComp = m_ecs->getComponent<SpriteComponent>(entity);
        
        if (transform && spriteComp && spriteComp->visible) {
            spriteComp->sprite.setPosition(transform->position);
            spriteComp->sprite.drawWithCamera(shader, m_camera.get(), 800, 600);
        }
    }
}

// ============================================================================
// PauseScene Implementation
// ============================================================================

PauseScene::PauseScene()
    : Scene("Pause")
    , m_selectedOption(0) {
}

void PauseScene::onEnter() {
    std::cout << "PauseScene: Entered" << std::endl;
    m_selectedOption = 0;
}

void PauseScene::onExit() {
    std::cout << "PauseScene: Exited" << std::endl;
}

void PauseScene::handleInput(Input& input) {
    if (input.isKeyJustPressed(KeyCode::Up)) {
        m_selectedOption = (m_selectedOption - 1 + 3) % 3;
    }
    if (input.isKeyJustPressed(KeyCode::Down)) {
        m_selectedOption = (m_selectedOption + 1) % 3;
    }
    if (input.isKeyJustPressed(KeyCode::Enter) || input.isKeyJustPressed(KeyCode::Space)) {
        if (m_selectedOption == 0) {
            // Resume
            m_sceneManager->popScene();
        } else if (m_selectedOption == 1) {
            // Restart
            m_sceneManager->changeScene("Game");
        } else if (m_selectedOption == 2) {
            // Main Menu
            m_sceneManager->changeScene("Menu");
        }
    }
    if (input.isKeyJustPressed(KeyCode::Escape)) {
        m_sceneManager->popScene();
    }
}

void PauseScene::update(float deltaTime) {
    // Pause doesn't update much
}

void PauseScene::render(Renderer& renderer) {
    // Semi-transparent overlay (in real game, render game scene below)
    renderer.clear(0.0f, 0.0f, 0.0f, 0.5f);
    
    std::cout << "\r[PAUSE] " << (m_selectedOption == 0 ? "> Resume" : "  Resume")
              << " | " << (m_selectedOption == 1 ? "> Restart" : "  Restart")
              << " | " << (m_selectedOption == 2 ? "> Main Menu" : "  Main Menu") << "          " << std::flush;
}
