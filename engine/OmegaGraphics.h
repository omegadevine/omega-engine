/**
 * OMEGA ENGINE - Graphics System
 * 
 * SDL2-based rendering abstraction for all Omega games.
 * Provides a simple, consistent API for 2D rendering that can
 * be extended to support 3D in the future.
 */

#ifndef OMEGA_GRAPHICS_H
#define OMEGA_GRAPHICS_H

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "OmegaMath.h"

namespace Omega {

// Forward declarations
class Texture;
class Font;

/**
 * Texture handle - represents a loaded image/texture
 */
class Texture {
    friend class Graphics;
private:
    SDL_Texture* sdlTexture;
    int width;
    int height;
    std::string name;
    
public:
    Texture() : sdlTexture(nullptr), width(0), height(0) {}
    ~Texture();
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Vec2i getSize() const { return Vec2i(width, height); }
    bool isValid() const { return sdlTexture != nullptr; }
    const std::string& getName() const { return name; }
};

/**
 * Sprite definition - a region of a texture
 */
struct Sprite {
    Texture* texture;
    Recti srcRect;      // Region within texture
    Vec2 origin;        // Origin point (for rotation), relative to sprite size (0-1)
    
    Sprite() : texture(nullptr), origin(0.5, 0.5) {}
    Sprite(Texture* tex) : texture(tex), origin(0.5, 0.5) {
        if (tex) srcRect = Recti(0, 0, tex->getWidth(), tex->getHeight());
    }
    Sprite(Texture* tex, const Recti& src, const Vec2& orig = Vec2(0.5, 0.5))
        : texture(tex), srcRect(src), origin(orig) {}
    
    int getWidth() const { return srcRect.w; }
    int getHeight() const { return srcRect.h; }
};

/**
 * Draw parameters for sprites
 */
struct DrawParams {
    Vec2 position;          // Where to draw
    Vec2 scale;             // Scale factor (1.0 = original size)
    double rotation;        // Rotation in radians
    Color tint;             // Color tint/modulation
    bool flipH;             // Flip horizontally
    bool flipV;             // Flip vertically
    
    DrawParams() : scale(1.0, 1.0), rotation(0), tint(Color::white()), 
                   flipH(false), flipV(false) {}
    
    DrawParams& at(double x, double y) { position = Vec2(x, y); return *this; }
    DrawParams& at(const Vec2& pos) { position = pos; return *this; }
    DrawParams& scaled(double s) { scale = Vec2(s, s); return *this; }
    DrawParams& scaled(double sx, double sy) { scale = Vec2(sx, sy); return *this; }
    DrawParams& rotated(double r) { rotation = r; return *this; }
    DrawParams& tinted(const Color& c) { tint = c; return *this; }
    DrawParams& flippedH() { flipH = true; return *this; }
    DrawParams& flippedV() { flipV = true; return *this; }
};

/**
 * Main graphics class - handles all rendering
 */
class Graphics {
    friend class Core;
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;
    
    // Texture cache
    std::unordered_map<std::string, std::unique_ptr<Texture>> textureCache;
    
    // Current render state
    Color currentColor;
    
    // Private constructor - created by Core
    Graphics();
    
    bool init(int width, int height, const std::string& title, bool fullscreen, bool vsync);
    void shutdown();
    
public:
    ~Graphics();
    
    // ========================================
    // FRAME MANAGEMENT
    // ========================================
    
    /**
     * Clear the screen with a color
     */
    void clear(const Color& color = Color::black());
    
    /**
     * Present the rendered frame to the screen
     */
    void present();
    
    // ========================================
    // STATE
    // ========================================
    
    /**
     * Set the current drawing color (for primitives)
     */
    void setColor(const Color& color);
    
    /**
     * Get screen dimensions
     */
    int getWidth() const { return screenWidth; }
    int getHeight() const { return screenHeight; }
    Vec2i getSize() const { return Vec2i(screenWidth, screenHeight); }
    
    // ========================================
    // PRIMITIVE DRAWING
    // ========================================
    
    /**
     * Draw a single pixel
     */
    void drawPixel(int x, int y);
    void drawPixel(int x, int y, const Color& color);
    
    /**
     * Draw a line
     */
    void drawLine(int x1, int y1, int x2, int y2);
    void drawLine(int x1, int y1, int x2, int y2, const Color& color);
    void drawLine(const Vec2& a, const Vec2& b, const Color& color);
    
    /**
     * Draw a rectangle outline
     */
    void drawRect(int x, int y, int w, int h);
    void drawRect(int x, int y, int w, int h, const Color& color);
    void drawRect(const Rect& rect, const Color& color);
    
    /**
     * Draw a filled rectangle
     */
    void fillRect(int x, int y, int w, int h);
    void fillRect(int x, int y, int w, int h, const Color& color);
    void fillRect(const Rect& rect, const Color& color);
    
    /**
     * Draw a circle outline
     */
    void drawCircle(int cx, int cy, int radius, const Color& color);
    
    /**
     * Draw a filled circle
     */
    void fillCircle(int cx, int cy, int radius, const Color& color);
    
    /**
     * Draw a polygon (filled trapezoid - useful for pseudo-3D roads)
     */
    void fillQuad(int x1, int y1, int x2, int y2, 
                  int x3, int y3, int x4, int y4, const Color& color);
    
    /**
     * Draw a horizontal gradient rectangle
     */
    void fillRectGradientH(int x, int y, int w, int h, 
                           const Color& left, const Color& right);
    
    /**
     * Draw a vertical gradient rectangle
     */
    void fillRectGradientV(int x, int y, int w, int h,
                           const Color& top, const Color& bottom);
    
    // ========================================
    // TEXTURE MANAGEMENT
    // ========================================
    
    /**
     * Load a texture from file (BMP supported natively, others need SDL_image)
     * Returns nullptr on failure
     */
    Texture* loadTexture(const std::string& filename);
    
    /**
     * Create a texture from raw pixel data
     * @param pixels RGBA pixel data (width * height * 4 bytes)
     */
    Texture* createTexture(int width, int height, const uint8_t* pixels);
    
    /**
     * Create an empty texture (render target)
     */
    Texture* createRenderTarget(int width, int height);
    
    /**
     * Unload a specific texture
     */
    void unloadTexture(const std::string& name);
    
    /**
     * Unload all textures
     */
    void unloadAllTextures();
    
    // ========================================
    // TEXTURE/SPRITE DRAWING
    // ========================================
    
    /**
     * Draw a texture at position
     */
    void drawTexture(Texture* texture, int x, int y);
    
    /**
     * Draw a texture with full parameters
     */
    void drawTexture(Texture* texture, const DrawParams& params);
    
    /**
     * Draw a portion of a texture
     */
    void drawTexture(Texture* texture, const Recti& src, const Rect& dst);
    
    /**
     * Draw a sprite
     */
    void drawSprite(const Sprite& sprite, int x, int y);
    void drawSprite(const Sprite& sprite, const DrawParams& params);
    
    // ========================================
    // RENDER TARGETS
    // ========================================
    
    /**
     * Set a texture as the render target (nullptr = screen)
     */
    void setRenderTarget(Texture* target);
    
    // ========================================
    // LOW-LEVEL ACCESS (for advanced use)
    // ========================================
    
    SDL_Renderer* getSDLRenderer() { return renderer; }
    SDL_Window* getSDLWindow() { return window; }
};

} // namespace Omega

#endif // OMEGA_GRAPHICS_H
