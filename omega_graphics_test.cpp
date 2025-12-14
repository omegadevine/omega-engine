/**
 * OMEGA ENGINE - Graphics Test Demo
 * 
 * Simple test to verify the graphics system works.
 * Demonstrates: primitives, colors, input, and basic animation.
 */

#include "engine/Omega.h"
#include <cmath>

using namespace Omega;

// Test state
double elapsedTime = 0;
Vec2 playerPos(640, 360);
double playerAngle = 0;
BitmapFont* font = nullptr;

void update(double dt) {
    elapsedTime += dt;
    
    // Get input
    Input* input = getCore()->getInput();
    
    // Move player
    Vec2 movement = input->getMovementVector();
    playerPos += movement * 300 * dt;
    
    // Rotate with Q/E
    if (input->isKeyDown(Key::Q)) playerAngle -= 3 * dt;
    if (input->isKeyDown(Key::E)) playerAngle += 3 * dt;
    
    // Keep player on screen
    playerPos.x = clamp(playerPos.x, 50.0, 1230.0);
    playerPos.y = clamp(playerPos.y, 50.0, 670.0);
}

void render() {
    Graphics* gfx = getCore()->getGraphics();
    
    // Clear to dark blue
    gfx->clear(Color(20, 30, 50));
    
    // Draw some gradient backgrounds
    gfx->fillRectGradientV(0, 0, 1280, 200, Color(50, 0, 80), Color(20, 30, 50));
    
    // Draw a "road" - demonstrates fillQuad for pseudo-3D
    int roadTop = 250;
    int roadBottom = 720;
    int roadWidthTop = 100;
    int roadWidthBottom = 600;
    int centerX = 640;
    
    gfx->fillQuad(
        centerX - roadWidthTop, roadTop,
        centerX + roadWidthTop, roadTop,
        centerX + roadWidthBottom, roadBottom,
        centerX - roadWidthBottom, roadBottom,
        Color(60, 60, 70)
    );
    
    // Road edges
    gfx->drawLine(centerX - roadWidthTop, roadTop, 
                  centerX - roadWidthBottom, roadBottom, Color::white());
    gfx->drawLine(centerX + roadWidthTop, roadTop,
                  centerX + roadWidthBottom, roadBottom, Color::white());
    
    // Center line dashes
    for (int i = 0; i < 10; i++) {
        float t = (float)i / 10 + fmod(elapsedTime * 0.3, 0.1f);
        int y = roadTop + (int)((roadBottom - roadTop) * t);
        int width = (int)(2 + t * 6);
        gfx->fillRect(centerX - width/2, y, width, 20 + (int)(t * 30), Color::yellow());
    }
    
    // Draw animated circles
    for (int i = 0; i < 5; i++) {
        double angle = elapsedTime + i * PI * 2 / 5;
        int x = 640 + (int)(cos(angle) * 200);
        int y = 150 + (int)(sin(angle) * 50);
        
        Color c = Color::fromFloat(
            0.5f + 0.5f * sin(elapsedTime + i),
            0.5f + 0.5f * cos(elapsedTime + i * 0.7f),
            0.5f + 0.5f * sin(elapsedTime * 0.5f + i),
            1.0f
        );
        
        gfx->fillCircle(x, y, 20 + i * 5, c);
        gfx->drawCircle(x, y, 25 + i * 5, Color::white());
    }
    
    // Draw player (simple car shape)
    int px = (int)playerPos.x;
    int py = (int)playerPos.y;
    
    // Car body
    gfx->fillRect(px - 20, py - 30, 40, 60, Color::red());
    gfx->fillRect(px - 15, py - 20, 30, 25, Color(150, 200, 255));  // Windshield
    
    // Wheels
    gfx->fillRect(px - 25, py - 25, 8, 20, Color(30, 30, 30));
    gfx->fillRect(px + 17, py - 25, 8, 20, Color(30, 30, 30));
    gfx->fillRect(px - 25, py + 10, 8, 20, Color(30, 30, 30));
    gfx->fillRect(px + 17, py + 10, 8, 20, Color(30, 30, 30));
    
    // Draw rectangles in corners to show screen bounds
    gfx->drawRect(10, 10, 100, 60, Color::green());
    gfx->drawRect(1170, 10, 100, 60, Color::green());
    gfx->drawRect(10, 650, 100, 60, Color::green());
    gfx->drawRect(1170, 650, 100, 60, Color::green());
    
    // Draw FPS with text
    int fps = getCore()->getFPS();
    font->drawText(20, 15, "FPS: " + std::to_string(fps), fps > 55 ? Color::green() : Color::red(), 2);
    
    // Instructions
    font->drawText(150, 15, "WASD to move, ESC to quit", Color::white(), 1);
    font->drawTextCentered(690, "OMEGA ENGINE GRAPHICS TEST", Color::yellow(), 1280, 2);
    
    // Input indicator
    Input* input = getCore()->getInput();
    int indicatorX = 20;
    int indicatorY = 50;
    
    // Show WASD state
    Color keyOn = Color::green();
    Color keyOff = Color(50, 50, 50);
    
    gfx->fillRect(indicatorX + 20, indicatorY, 16, 16, 
                  input->isKeyDown(Key::W) ? keyOn : keyOff);  // W
    gfx->fillRect(indicatorX, indicatorY + 20, 16, 16,
                  input->isKeyDown(Key::A) ? keyOn : keyOff);  // A
    gfx->fillRect(indicatorX + 20, indicatorY + 20, 16, 16,
                  input->isKeyDown(Key::S) ? keyOn : keyOff);  // S
    gfx->fillRect(indicatorX + 40, indicatorY + 20, 16, 16,
                  input->isKeyDown(Key::D) ? keyOn : keyOff);  // D
    
    // Present frame
    gfx->present();
}

int main(int argc, char* argv[]) {
    // Configure engine
    EngineConfig config;
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.windowTitle = "Omega Engine - Graphics Test";
    config.vsync = true;
    
    // Initialize
    if (!getCore()->init(config)) {
        return 1;
    }
    
    // Create font
    font = new BitmapFont(getCore()->getGraphics());
    
    // Run game loop
    getCore()->run(update, render);
    
    // Cleanup
    delete font;
    Core::destroyInstance();
    
    return 0;
}
