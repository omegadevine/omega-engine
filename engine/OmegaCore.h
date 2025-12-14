/**
 * OMEGA ENGINE - Core System
 * 
 * Provides initialization, shutdown, and main loop timing for all Omega games.
 * This is the foundation that all game modules build upon.
 */

#ifndef OMEGA_CORE_H
#define OMEGA_CORE_H

#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include <chrono>

namespace Omega {

// Forward declarations
class Graphics;
class Input;

/**
 * Core engine configuration
 */
struct EngineConfig {
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string windowTitle = "Omega Engine";
    bool fullscreen = false;
    bool vsync = true;
    int targetFPS = 60;         // 0 = unlimited
    bool showFPS = false;
};

/**
 * Core engine class - manages lifecycle and main loop
 */
class Core {
private:
    static Core* instance;
    
    bool initialized;
    bool running;
    EngineConfig config;
    
    // Timing
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    double deltaTime;           // Seconds since last frame
    double totalTime;           // Total elapsed time
    int frameCount;
    double fpsTimer;
    int currentFPS;
    
    // Subsystems (owned by Core)
    Graphics* graphics;
    Input* input;
    
    Core();
    
public:
    ~Core();
    
    // Singleton access
    static Core* getInstance();
    static void destroyInstance();
    
    /**
     * Initialize the engine with the given configuration
     * Returns true on success
     */
    bool init(const EngineConfig& cfg = EngineConfig());
    
    /**
     * Shutdown the engine and release all resources
     */
    void shutdown();
    
    /**
     * Check if engine is initialized and running
     */
    bool isRunning() const { return running && initialized; }
    
    /**
     * Signal the engine to stop
     */
    void quit() { running = false; }
    
    /**
     * Run the main loop with callbacks
     * @param updateFunc Called each frame with delta time (seconds)
     * @param renderFunc Called each frame for rendering
     */
    void run(std::function<void(double)> updateFunc, 
             std::function<void()> renderFunc);
    
    /**
     * Process a single frame (for custom main loops)
     * Returns false if engine should quit
     */
    bool processFrame(std::function<void(double)> updateFunc,
                      std::function<void()> renderFunc);
    
    // Accessors
    double getDeltaTime() const { return deltaTime; }
    double getTotalTime() const { return totalTime; }
    int getFPS() const { return currentFPS; }
    const EngineConfig& getConfig() const { return config; }
    
    Graphics* getGraphics() { return graphics; }
    Input* getInput() { return input; }
};

// Convenience function to get the core instance
inline Core* getCore() { return Core::getInstance(); }

} // namespace Omega

#endif // OMEGA_CORE_H
