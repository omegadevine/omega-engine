#include "Debug.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

// ============================================================================
// DebugRenderer Implementation
// ============================================================================

DebugRenderer::DebugRenderer()
    : m_enabled(true) {
}

DebugRenderer& DebugRenderer::getInstance() {
    static DebugRenderer instance;
    return instance;
}

void DebugRenderer::drawLine(const Vector2& start, const Vector2& end, const Color& color, float lifetime) {
    m_lines.push_back({start, end, color, lifetime});
}

void DebugRenderer::drawRect(const Vector2& pos, const Vector2& size, const Color& color, bool filled, float lifetime) {
    m_rects.push_back({pos, size, color, lifetime, filled});
}

void DebugRenderer::drawCircle(const Vector2& center, float radius, const Color& color, bool filled, float lifetime) {
    m_circles.push_back({center, radius, color, lifetime, filled});
}

void DebugRenderer::drawText(const std::string& text, const Vector2& pos, const Color& color, float scale, float lifetime) {
    m_texts.push_back({text, pos, color, lifetime, scale});
}

void DebugRenderer::drawCross(const Vector2& pos, float size, const Color& color, float lifetime) {
    drawLine(Vector2(pos.x - size, pos.y), Vector2(pos.x + size, pos.y), color, lifetime);
    drawLine(Vector2(pos.x, pos.y - size), Vector2(pos.x, pos.y + size), color, lifetime);
}

void DebugRenderer::drawGrid(const Vector2& origin, int width, int height, int cellSize, const Color& color) {
    // Vertical lines
    for (int x = 0; x <= width; x += cellSize) {
        drawLine(
            Vector2(origin.x + x, origin.y),
            Vector2(origin.x + x, origin.y + height),
            color
        );
    }
    
    // Horizontal lines
    for (int y = 0; y <= height; y += cellSize) {
        drawLine(
            Vector2(origin.x, origin.y + y),
            Vector2(origin.x + width, origin.y + y),
            color
        );
    }
}

void DebugRenderer::drawAABB(const AABB& aabb, const Color& color, float lifetime) {
    drawRect(Vector2(aabb.minX, aabb.minY), Vector2(aabb.maxX - aabb.minX, aabb.maxY - aabb.minY), color, false, lifetime);
}

void DebugRenderer::drawCircleCollider(const CircleCollider& circle, const Color& color, float lifetime) {
    drawCircle(circle.center, circle.radius, color, false, lifetime);
    drawCross(circle.center, 5.0f, color, lifetime);
}

void DebugRenderer::drawCamera(const Camera* camera, int screenWidth, int screenHeight, const Color& color) {
    if (!camera) return;
    
    Vector2 pos = camera->getPosition();
    drawRect(pos, Vector2(screenWidth, screenHeight), color, false);
    drawCross(pos, 20.0f, color);
}

void DebugRenderer::update(float deltaTime) {
    // Update lifetimes and remove expired shapes
    m_lines.erase(
        std::remove_if(m_lines.begin(), m_lines.end(), [deltaTime](DebugLine& line) {
            if (line.lifetime > 0.0f) {
                line.lifetime -= deltaTime;
                return line.lifetime <= 0.0f;
            }
            return false;
        }),
        m_lines.end()
    );
    
    m_rects.erase(
        std::remove_if(m_rects.begin(), m_rects.end(), [deltaTime](DebugRect& rect) {
            if (rect.lifetime > 0.0f) {
                rect.lifetime -= deltaTime;
                return rect.lifetime <= 0.0f;
            }
            return false;
        }),
        m_rects.end()
    );
    
    m_circles.erase(
        std::remove_if(m_circles.begin(), m_circles.end(), [deltaTime](DebugCircle& circle) {
            if (circle.lifetime > 0.0f) {
                circle.lifetime -= deltaTime;
                return circle.lifetime <= 0.0f;
            }
            return false;
        }),
        m_circles.end()
    );
    
    m_texts.erase(
        std::remove_if(m_texts.begin(), m_texts.end(), [deltaTime](DebugText& text) {
            if (text.lifetime > 0.0f) {
                text.lifetime -= deltaTime;
                return text.lifetime <= 0.0f;
            }
            return false;
        }),
        m_texts.end()
    );
}

void DebugRenderer::renderLine(const Vector2& start, const Vector2& end, const Color& color, Shader* shader, int screenWidth, int screenHeight) {
    Sprite lineSprite;
    lineSprite.setColor(color);
    
    // Calculate line properties
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float length = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx) * 180.0f / 3.14159f;
    
    lineSprite.setPosition(start);
    lineSprite.setSize(Vector2(length, 2.0f));
    lineSprite.setRotation(angle);
    lineSprite.draw(shader, screenWidth, screenHeight);
}

void DebugRenderer::renderCircleOutline(const Vector2& center, float radius, const Color& color, Shader* shader, int screenWidth, int screenHeight) {
    const int segments = 32;
    const float angleStep = (2.0f * 3.14159f) / segments;
    
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
        
        Vector2 p1(center.x + std::cos(angle1) * radius, center.y + std::sin(angle1) * radius);
        Vector2 p2(center.x + std::cos(angle2) * radius, center.y + std::sin(angle2) * radius);
        
        renderLine(p1, p2, color, shader, screenWidth, screenHeight);
    }
}

void DebugRenderer::render(Shader* shader, int screenWidth, int screenHeight) {
    if (!m_enabled) return;
    
    // Render lines
    for (const auto& line : m_lines) {
        renderLine(line.start, line.end, line.color, shader, screenWidth, screenHeight);
    }
    
    // Render rects
    for (const auto& rect : m_rects) {
        if (rect.filled) {
            Sprite sprite;
            sprite.setPosition(rect.position);
            sprite.setSize(rect.size);
            sprite.setColor(rect.color);
            sprite.draw(shader, screenWidth, screenHeight);
        } else {
            // Draw 4 lines for outline
            Vector2 tl = rect.position;
            Vector2 tr(rect.position.x + rect.size.x, rect.position.y);
            Vector2 br(rect.position.x + rect.size.x, rect.position.y + rect.size.y);
            Vector2 bl(rect.position.x, rect.position.y + rect.size.y);
            
            renderLine(tl, tr, rect.color, shader, screenWidth, screenHeight);
            renderLine(tr, br, rect.color, shader, screenWidth, screenHeight);
            renderLine(br, bl, rect.color, shader, screenWidth, screenHeight);
            renderLine(bl, tl, rect.color, shader, screenWidth, screenHeight);
        }
    }
    
    // Render circles
    for (const auto& circle : m_circles) {
        if (circle.filled) {
            Sprite sprite;
            sprite.setPosition(Vector2(circle.center.x - circle.radius, circle.center.y - circle.radius));
            sprite.setSize(Vector2(circle.radius * 2, circle.radius * 2));
            sprite.setColor(circle.color);
            sprite.draw(shader, screenWidth, screenHeight);
        } else {
            renderCircleOutline(circle.center, circle.radius, circle.color, shader, screenWidth, screenHeight);
        }
    }
    
    // Text rendering would go here (requires Text system)
    // For now, texts are stored but not rendered
}

void DebugRenderer::clear() {
    m_lines.clear();
    m_rects.clear();
    m_circles.clear();
    m_texts.clear();
}

// ============================================================================
// Profiler Implementation
// ============================================================================

Profiler::Profiler()
    : m_enabled(true) {
    m_stats = {};
}

Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::beginFrame() {
    m_frameStart = std::chrono::high_resolution_clock::now();
}

void Profiler::endFrame() {
    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - m_frameStart);
    
    m_stats.frameTime = frameDuration.count() / 1000.0f; // Convert to milliseconds
    m_stats.fps = (m_stats.frameTime > 0.0f) ? 1000.0f / m_stats.frameTime : 0.0f;
}

void Profiler::beginSection(const std::string& name) {
    m_sectionStarts[name] = std::chrono::high_resolution_clock::now();
}

void Profiler::endSection(const std::string& name) {
    auto it = m_sectionStarts.find(name);
    if (it != m_sectionStarts.end()) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second);
        m_sectionTimes[name] = duration.count() / 1000.0f; // Convert to milliseconds
    }
}

float Profiler::getSectionTime(const std::string& name) const {
    auto it = m_sectionTimes.find(name);
    return (it != m_sectionTimes.end()) ? it->second : 0.0f;
}

void Profiler::renderStats(DebugRenderer* debugRenderer, const Vector2& position) {
    if (!m_enabled || !debugRenderer) return;
    
    // Draw background
    debugRenderer->drawRect(position, Vector2(200, 150), Color(0, 0, 0, 0.7f), true);
    
    // Draw stats as text (placeholder - would need Text system)
    float yOffset = position.y + 10;
    const float lineHeight = 15.0f;
    
    // Note: Actual text rendering would require the Text system
    // For now, we just prepare the strings
    char buffer[256];
    std::vector<std::string> lines;
    
    snprintf(buffer, sizeof(buffer), "FPS: %.1f", m_stats.fps);
    lines.push_back(buffer);
    
    snprintf(buffer, sizeof(buffer), "Frame: %.2fms", m_stats.frameTime);
    lines.push_back(buffer);
    
    snprintf(buffer, sizeof(buffer), "Draw Calls: %d", m_stats.drawCalls);
    lines.push_back(buffer);
    
    snprintf(buffer, sizeof(buffer), "Entities: %d", m_stats.entityCount);
    lines.push_back(buffer);
    
    snprintf(buffer, sizeof(buffer), "Particles: %d", m_stats.particleCount);
    lines.push_back(buffer);
    
    snprintf(buffer, sizeof(buffer), "Memory: %.2f MB", m_stats.memoryUsage / (1024.0f * 1024.0f));
    lines.push_back(buffer);
    
    // Store as debug text
    for (const auto& line : lines) {
        debugRenderer->drawText(line, Vector2(position.x + 10, yOffset), Color(1, 1, 1, 1), 1.0f);
        yOffset += lineHeight;
    }
}

// ============================================================================
// DebugConsole Implementation
// ============================================================================

DebugConsole::DebugConsole()
    : m_enabled(false)
    , m_maxMessages(100) {
}

DebugConsole& DebugConsole::getInstance() {
    static DebugConsole instance;
    return instance;
}

void DebugConsole::log(const std::string& message) {
    LogMessage msg;
    msg.text = message;
    msg.color = Color(1, 1, 1, 1);
    msg.timestamp = std::chrono::system_clock::now();
    
    m_messages.push_back(msg);
    
    // Also print to stdout
    std::cout << "[LOG] " << message << std::endl;
    
    // Limit message count
    if (m_messages.size() > m_maxMessages) {
        m_messages.erase(m_messages.begin());
    }
}

void DebugConsole::logWarning(const std::string& message) {
    LogMessage msg;
    msg.text = "[WARNING] " + message;
    msg.color = Color(1, 1, 0, 1);
    msg.timestamp = std::chrono::system_clock::now();
    
    m_messages.push_back(msg);
    std::cout << "[WARNING] " << message << std::endl;
    
    if (m_messages.size() > m_maxMessages) {
        m_messages.erase(m_messages.begin());
    }
}

void DebugConsole::logError(const std::string& message) {
    LogMessage msg;
    msg.text = "[ERROR] " + message;
    msg.color = Color(1, 0, 0, 1);
    msg.timestamp = std::chrono::system_clock::now();
    
    m_messages.push_back(msg);
    std::cerr << "[ERROR] " << message << std::endl;
    
    if (m_messages.size() > m_maxMessages) {
        m_messages.erase(m_messages.begin());
    }
}

void DebugConsole::registerCommand(const std::string& name, std::function<void(const std::vector<std::string>&)> callback) {
    m_commands[name] = callback;
    log("Registered command: " + name);
}

void DebugConsole::executeCommand(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.empty()) return;
    
    std::string cmdName = tokens[0];
    tokens.erase(tokens.begin());
    
    auto it = m_commands.find(cmdName);
    if (it != m_commands.end()) {
        log("Executing: " + command);
        it->second(tokens);
    } else {
        logError("Unknown command: " + cmdName);
    }
}

void DebugConsole::render(DebugRenderer* debugRenderer, const Vector2& position) {
    if (!m_enabled || !debugRenderer) return;
    
    // Draw console background
    debugRenderer->drawRect(position, Vector2(600, 300), Color(0, 0, 0, 0.8f), true);
    
    // Render messages (would need Text system)
    float yOffset = position.y + 10;
    const float lineHeight = 15.0f;
    const int maxVisibleMessages = 18;
    
    int startIdx = std::max(0, static_cast<int>(m_messages.size()) - maxVisibleMessages);
    
    for (size_t i = startIdx; i < m_messages.size(); i++) {
        const auto& msg = m_messages[i];
        debugRenderer->drawText(msg.text, Vector2(position.x + 10, yOffset), msg.color, 1.0f);
        yOffset += lineHeight;
    }
}

#endif // OMEGA_DEBUG_H
