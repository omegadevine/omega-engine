#include <SDL.h>
#include <iostream>
#include <cstring>
#include <cmath>
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Sprite.h"
#include "ECS.h"
#include "Input.h"
#include "AssetManager.h"
#include "Camera.h"
#include "AnimatedSprite.h"
#include "Animation.h"
#include "Collision.h"

int main(int argc, char** argv) {
    // Input validation for command line arguments
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (argv[i] && std::strlen(argv[i]) > 1024) {
                std::cerr << "Error: Command line argument too long" << std::endl;
                return 1;
            }
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    SDL_Window* window = SDL_CreateWindow(
        "omega-engine - Collision Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Initialize renderer
    Renderer renderer(window);
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Get Asset Manager instance
    AssetManager& assets = AssetManager::getInstance();

    // Load sprite shader using Asset Manager
    const std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        
        uniform vec2 position;
        uniform vec2 size;
        
        out vec2 TexCoord;
        
        void main() {
            vec2 scaledPos = aPos * size + position;
            gl_Position = vec4(scaledPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const std::string fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        
        uniform sampler2D image;
        uniform vec4 spriteColor;
        
        void main() {
            FragColor = texture(image, TexCoord) * spriteColor;
        }
    )";

    Shader* spriteShader = assets.loadShader("sprite_shader", vertexShaderSource, fragmentShaderSource);
    if (!spriteShader) {
        std::cerr << "Failed to load sprite shader" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load texture using Asset Manager
    Texture* testTexture = assets.loadTexture("test_pattern", "test.png");
    if (!testTexture) {
        std::cerr << "Warning: Failed to load texture, continuing anyway" << std::endl;
    }

    // Create ECS
    ECS ecs;

    // Create Camera
    Camera camera(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));
    camera.setFollowSpeed(3.0f);
    camera.setBounds(0, 0, 1600, 1200); // World is 2x screen size

    // Create animated player sprite
    AnimatedSprite playerAnimSprite;
    playerAnimSprite.setTexture(testTexture);
    playerAnimSprite.setSize(Vector2(64, 64));
    playerAnimSprite.setColor(Color(0.3f, 0.7f, 1.0f, 1.0f));
    
    // Create animations (simple example with procedural frames)
    Animation idleAnim("idle");
    idleAnim.setFrameDuration(0.2f);
    idleAnim.setLoop(true);
    for (int i = 0; i < 4; i++) {
        idleAnim.addFrame(i * 64, 0, 64, 64); // 4 frames horizontally
    }
    playerAnimSprite.addAnimation("idle", idleAnim);
    
    Animation walkAnim("walk");
    walkAnim.setFrameDuration(0.1f);
    walkAnim.setLoop(true);
    for (int i = 0; i < 6; i++) {
        walkAnim.addFrame(i * 64, 64, 64, 64); // 6 frames on second row
    }
    playerAnimSprite.addAnimation("walk", walkAnim);
    
    playerAnimSprite.play("idle");

    // Create player entity
    Entity player = ecs.createEntity();
    auto* playerTransform = ecs.addComponent<Transform>(player);
    playerTransform->position = Vector2(SCREEN_WIDTH / 2.0f - 32, SCREEN_HEIGHT / 2.0f - 32);
    playerTransform->scale = Vector2(1.0f, 1.0f);
    
    auto* playerSprite = ecs.addComponent<SpriteComponent>(player);
    playerSprite->sprite.setTexture(testTexture);
    playerSprite->sprite.setSize(Vector2(64, 64));
    playerSprite->sprite.setColor(Color(0.3f, 0.7f, 1.0f, 1.0f));
    
    // Add player collider
    auto* playerCollider = ecs.addComponent<Collider>(player);
    playerCollider->type = ColliderType::Box;
    playerCollider->size = Vector2(64, 64);
    playerCollider->layer = 1;
    playerCollider->mask = 0xFFFFFFFF;
    playerCollider->isTrigger = false;
    playerCollider->isStatic = false;
    playerCollider->onCollisionEnter = [](Entity other) {
        std::cout << "Player collided with entity " << other << std::endl;
    };

    // Create some floating entities with colliders
    std::vector<Entity> floaters;
    for (int i = 0; i < 5; i++) {
        Entity floater = ecs.createEntity();
        auto* transform = ecs.addComponent<Transform>(floater);
        transform->position = Vector2(100.0f + i * 120.0f, 100.0f + (i % 2) * 100.0f);
        
        auto* sprite = ecs.addComponent<SpriteComponent>(floater);
        sprite->sprite.setTexture(testTexture);
        sprite->sprite.setSize(Vector2(48, 48));
        
        float hue = i / 5.0f;
        sprite->sprite.setColor(Color(
            0.5f + 0.5f * std::sin(hue * 6.28f),
            0.5f + 0.5f * std::sin((hue + 0.33f) * 6.28f),
            0.5f + 0.5f * std::sin((hue + 0.66f) * 6.28f),
            1.0f
        ));
        
        // Add colliders (alternating box and circle)
        auto* collider = ecs.addComponent<Collider>(floater);
        if (i % 2 == 0) {
            collider->type = ColliderType::Box;
            collider->size = Vector2(48, 48);
        } else {
            collider->type = ColliderType::Circle;
            collider->size = Vector2(24, 24); // radius in x
        }
        collider->layer = 2;
        collider->mask = 0xFFFFFFFF;
        collider->isTrigger = false;
        collider->isStatic = true; // These don't move
        
        floaters.push_back(floater);
    }
    
    // Create collision system
    CollisionSystem collisionSystem(&ecs);

    bool running = true;
    SDL_Event event;
    float time = 0.0f;
    float deltaTime = 0.016f; // ~60fps
    bool isMoving = false;

    std::cout << "=== omega-engine Collision Demo ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD / Arrow Keys - Move player (with collision)" << std::endl;
    std::cout << "  Q/E - Zoom Out/In" << std::endl;
    std::cout << "  SPACE - Screen Shake" << std::endl;
    std::cout << "  R - Reset Camera" << std::endl;
    std::cout << "  ESC - Quit" << std::endl;
    std::cout << "Entities: " << ecs.getEntities().size() << std::endl;
    std::cout << "Loaded Textures: " << assets.getTextureCount() << std::endl;
    std::cout << "Loaded Shaders: " << assets.getShaderCount() << std::endl;
    std::cout << "World Size: 1600x1200 (camera follows player)" << std::endl;
    std::cout << "Animation: Idle <-> Walk (automatic)" << std::endl;
    std::cout << "Collision: AABB and Circle detection active" << std::endl;

    while (running) {
        Input& input = Input::getInstance();
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            input.update(event);
        }

        if (input.isKeyPressed(KeyCode::Escape)) {
            running = false;
        }

        // Camera controls
        if (input.isKeyJustPressed(KeyCode::Space)) {
            camera.shake(20.0f, 0.3f); // Shake with intensity 20 for 0.3 seconds
        }
        if (input.isKeyPressed(KeyCode::Q)) {
            camera.zoomOut(0.01f);
        }
        if (input.isKeyPressed(KeyCode::E)) {
            camera.zoomIn(0.01f);
        }
        if (input.isKeyJustPressed(KeyCode::R)) {
            camera.reset();
        }

        // Update player movement (now in world space)
        const float moveSpeed = 3.0f;
        isMoving = false;
        
        if (input.isKeyPressed(KeyCode::W) || input.isKeyPressed(KeyCode::Up)) {
            playerTransform->position.y -= moveSpeed;
            isMoving = true;
        }
        if (input.isKeyPressed(KeyCode::S) || input.isKeyPressed(KeyCode::Down)) {
            playerTransform->position.y += moveSpeed;
            isMoving = true;
        }
        if (input.isKeyPressed(KeyCode::A) || input.isKeyPressed(KeyCode::Left)) {
            playerTransform->position.x -= moveSpeed;
            isMoving = true;
        }
        if (input.isKeyPressed(KeyCode::D) || input.isKeyPressed(KeyCode::Right)) {
            playerTransform->position.x += moveSpeed;
            isMoving = true;
        }
        
        // Switch animations based on movement
        if (isMoving) {
            if (playerAnimSprite.getCurrentAnimation() != "walk") {
                playerAnimSprite.play("walk");
            }
        } else {
            if (playerAnimSprite.getCurrentAnimation() != "idle") {
                playerAnimSprite.play("idle");
            }
        }

        // Keep player in world bounds (not screen bounds!)
        if (playerTransform->position.x < 0) playerTransform->position.x = 0;
        if (playerTransform->position.y < 0) playerTransform->position.y = 0;
        if (playerTransform->position.x > 1600 - 64) 
            playerTransform->position.x = 1600 - 64;
        if (playerTransform->position.y > 1200 - 64) 
            playerTransform->position.y = 1200 - 64;

        // Camera follows player
        camera.follow(Vector2(
            playerTransform->position.x + 32, // Center on player
            playerTransform->position.y + 32
        ));
        camera.update(deltaTime);
        
        // Update animated sprite
        playerAnimSprite.setPosition(playerTransform->position);
        playerAnimSprite.update(deltaTime);
        
        // Update collision system
        collisionSystem.update();

        // Update floaters (sine wave animation)
        time += 0.02f;
        for (size_t i = 0; i < floaters.size(); i++) {
            auto* transform = ecs.getComponent<Transform>(floaters[i]);
            if (transform) {
                transform->position.y = 100.0f + (i % 2) * 100.0f + 
                    std::sin(time + i) * 30.0f;
            }
        }

        // Render
        renderer.clear(0.1f, 0.1f, 0.15f, 1.0f);

        // Render animated player sprite
        playerAnimSprite.drawWithCamera(spriteShader, &camera, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // Render other sprites using camera
        for (Entity entity : ecs.getEntities()) {
            auto* transform = ecs.getComponent<Transform>(entity);
            auto* spriteComp = ecs.getComponent<SpriteComponent>(entity);
            
            if (transform && spriteComp && spriteComp->visible) {
                // Skip player entity since we're rendering it with animation
                if (entity == player) continue;
                
                spriteComp->sprite.setPosition(transform->position);
                spriteComp->sprite.drawWithCamera(spriteShader, &camera, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
        }

        renderer.present();
        input.endFrame();

        SDL_Delay(16); // ~60fps
    }

    std::cout << "Shutting down..." << std::endl;
    
    // Clean up assets (automatic via AssetManager singleton)
    assets.unloadAll();

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
