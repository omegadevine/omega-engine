#ifndef OMEGA_DEBUG_H
#define OMEGA_DEBUG_H

#include "Sprite.h"
#include "Shader.h"
#include "Collision.h"
#include "Camera.h"
#include <vector>
#include <string>

// Debug draw shapes
struct DebugLine {
    Vector2 start;
    Vector2 end;
    Color color;
    float lifetime;
};

struct DebugRect {
    Vector2 position;
    Vector2 size;
    Color color;
    float lifetime;
    bool filled;
};

struct DebugCircle {
    Vector2 center;
    float radius;
    Color color;
    float lifetime;
    bool filled;
};

struct DebugText {
    std::string text;
    Vector2 position;
    Color color;
    float lifetime;
    float scale;
};

// Debug visualization system
class DebugRenderer {
public:
    static DebugRenderer& getInstance();
    
    // Drawing functions
    void drawLine(const Vector2& start, const Vector2& end, const Color& color = Color(1, 1, 1, 1), float lifetime = 0.0f);
    void drawRect(const Vector2& pos, const Vector2& size, const Color& color = Color(1, 1, 1, 1), bool filled = false, float lifetime = 0.0f);
    void drawCircle(const Vector2& center, float radius, const Color& color = Color(1, 1, 1, 1), bool filled = false, float lifetime = 0.0f);
    void drawText(const std::string& text, const Vector2& pos, const Color& color = Color(1, 1, 1, 1), float scale = 1.0f, float lifetime = 0.0f);
    void drawCross(const Vector2& pos, float size, const Color& color = Color(1, 1, 1, 1), float lifetime = 0.0f);
    void drawGrid(const Vector2& origin, int width, int height, int cellSize, const Color& color = Color(0.5f, 0.5f, 0.5f, 0.5f));
    
    // Collision visualization
    void drawAABB(const AABB& aabb, const Color& color = Color(0, 1, 0, 1), float lifetime = 0.0f);
    void drawCircleCollider(const CircleCollider& circle, const Color& color = Color(0, 1, 1, 1), float lifetime = 0.0f);
    
    // Camera visualization
    void drawCamera(const Camera* camera, int screenWidth, int screenHeight, const Color& color = Color(1, 1, 0, 1));
    
    // Update and render
    void update(float deltaTime);
    void render(Shader* shader, int screenWidth, int screenHeight);
    void clear();
    
    // Settings
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void toggleEnabled() { m_enabled = !m_enabled; }

private:
    DebugRenderer();
    ~DebugRenderer() = default;
    DebugRenderer(const DebugRenderer&) = delete;
    DebugRenderer& operator=(const DebugRenderer&) = delete;
    
    void renderLine(const Vector2& start, const Vector2& end, const Color& color, Shader* shader, int screenWidth, int screenHeight);
    void renderCircleOutline(const Vector2& center, float radius, const Color& color, Shader* shader, int screenWidth, int screenHeight);
    
    std::vector<DebugLine> m_lines;
    std::vector<DebugRect> m_rects;
    std::vector<DebugCircle> m_circles;
    std::vector<DebugText> m_texts;
    bool m_enabled;
};

// Performance statistics
struct PerformanceStats {
    float fps;
    float frameTime;
    float updateTime;
    float renderTime;
    int drawCalls;
    int entityCount;
    int particleCount;
    size_t memoryUsage;
};

// Performance profiler
class Profiler {
public:
    static Profiler& getInstance();
    
    void beginFrame();
    void endFrame();
    
    void beginSection(const std::string& name);
    void endSection(const std::string& name);
    
    void setDrawCalls(int count) { m_stats.drawCalls = count; }
    void setEntityCount(int count) { m_stats.entityCount = count; }
    void setParticleCount(int count) { m_stats.particleCount = count; }
    void setMemoryUsage(size_t bytes) { m_stats.memoryUsage = bytes; }
    
    const PerformanceStats& getStats() const { return m_stats; }
    float getSectionTime(const std::string& name) const;
    
    void renderStats(DebugRenderer* debugRenderer, const Vector2& position);
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void toggleEnabled() { m_enabled = !m_enabled; }

private:
    Profiler();
    ~Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
    
    PerformanceStats m_stats;
    std::unordered_map<std::string, float> m_sectionTimes;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_sectionStarts;
    std::chrono::high_resolution_clock::time_point m_frameStart;
    std::chrono::high_resolution_clock::time_point m_updateStart;
    std::chrono::high_resolution_clock::time_point m_renderStart;
    bool m_enabled;
};

// Console for logging and commands
class DebugConsole {
public:
    static DebugConsole& getInstance();
    
    void log(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
    
    void registerCommand(const std::string& name, std::function<void(const std::vector<std::string>&)> callback);
    void executeCommand(const std::string& command);
    
    void render(DebugRenderer* debugRenderer, const Vector2& position);
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void toggleEnabled() { m_enabled = !m_enabled; }
    
    void clear() { m_messages.clear(); }
    size_t getMessageCount() const { return m_messages.size(); }

private:
    DebugConsole();
    ~DebugConsole() = default;
    DebugConsole(const DebugConsole&) = delete;
    DebugConsole& operator=(const DebugConsole&) = delete;
    
    struct LogMessage {
        std::string text;
        Color color;
        std::chrono::system_clock::time_point timestamp;
    };
    
    std::vector<LogMessage> m_messages;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> m_commands;
    bool m_enabled;
    size_t m_maxMessages;
};

#endif // OMEGA_DEBUG_H
