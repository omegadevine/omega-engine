/**
 * OMEGA ENGINE - Core System Implementation
 */

#include "OmegaCore.h"
#include "OmegaGraphics.h"
#include "OmegaInput.h"
#include <iostream>

namespace Omega {

// Static instance
Core* Core::instance = nullptr;

Core::Core() 
    : initialized(false), running(false),
      deltaTime(0), totalTime(0), frameCount(0),
      fpsTimer(0), currentFPS(0),
      graphics(nullptr), input(nullptr) {
}

Core::~Core() {
    shutdown();
}

Core* Core::getInstance() {
    if (!instance) {
        instance = new Core();
    }
    return instance;
}

void Core::destroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

bool Core::init(const EngineConfig& cfg) {
    if (initialized) {
        std::cerr << "Omega Engine already initialized!" << std::endl;
        return false;
    }
    
    config = cfg;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create graphics subsystem
    graphics = new Graphics();
    if (!graphics->init(config.windowWidth, config.windowHeight, 
                        config.windowTitle, config.fullscreen, config.vsync)) {
        std::cerr << "Graphics initialization failed!" << std::endl;
        delete graphics;
        graphics = nullptr;
        SDL_Quit();
        return false;
    }
    
    // Create input subsystem
    input = new Input();
    if (!input->init()) {
        std::cerr << "Input initialization failed!" << std::endl;
        delete input;
        input = nullptr;
        delete graphics;
        graphics = nullptr;
        SDL_Quit();
        return false;
    }
    
    // Initialize timing
    lastFrameTime = std::chrono::high_resolution_clock::now();
    deltaTime = 0;
    totalTime = 0;
    frameCount = 0;
    fpsTimer = 0;
    currentFPS = 0;
    
    initialized = true;
    running = true;
    
    std::cout << "Omega Engine initialized successfully!" << std::endl;
    return true;
}

void Core::shutdown() {
    if (!initialized) return;
    
    if (input) {
        input->shutdown();
        delete input;
        input = nullptr;
    }
    
    if (graphics) {
        graphics->shutdown();
        delete graphics;
        graphics = nullptr;
    }
    
    SDL_Quit();
    
    initialized = false;
    running = false;
    
    std::cout << "Omega Engine shut down." << std::endl;
}

void Core::run(std::function<void(double)> updateFunc,
               std::function<void()> renderFunc) {
    while (processFrame(updateFunc, renderFunc)) {
        // Frame limiting if not using vsync
        if (config.targetFPS > 0 && !config.vsync) {
            double targetFrameTime = 1.0 / config.targetFPS;
            auto frameEnd = std::chrono::high_resolution_clock::now();
            double frameTime = std::chrono::duration<double>(
                frameEnd - lastFrameTime).count();
            
            if (frameTime < targetFrameTime) {
                SDL_Delay((Uint32)((targetFrameTime - frameTime) * 1000));
            }
        }
    }
}

bool Core::processFrame(std::function<void(double)> updateFunc,
                        std::function<void()> renderFunc) {
    if (!running) return false;
    
    // Calculate delta time
    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();
    lastFrameTime = currentTime;
    
    // Clamp delta time to prevent spiral of death
    if (deltaTime > 0.1) deltaTime = 0.1;
    
    totalTime += deltaTime;
    frameCount++;
    
    // FPS calculation
    fpsTimer += deltaTime;
    if (fpsTimer >= 1.0) {
        currentFPS = frameCount;
        frameCount = 0;
        fpsTimer -= 1.0;
        
        if (config.showFPS) {
            std::cout << "FPS: " << currentFPS << std::endl;
        }
    }
    
    // Update input state
    input->update();
    
    // Process SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        input->processEvent(event);
        
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                return false;
                
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // ESC quits by default (games can override)
                    running = false;
                    return false;
                }
                break;
        }
    }
    
    // Update game
    if (updateFunc) {
        updateFunc(deltaTime);
    }
    
    // Render game
    if (renderFunc) {
        renderFunc();
    }
    
    return running;
}

} // namespace Omega
