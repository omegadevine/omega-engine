/**
 * OMEGA ENGINE - Graphics System Implementation
 */

#include "OmegaGraphics.h"
#include <iostream>
#include <cstring>

namespace Omega {

// ============================================================
// TEXTURE
// ============================================================

Texture::~Texture() {
    if (sdlTexture) {
        SDL_DestroyTexture(sdlTexture);
        sdlTexture = nullptr;
    }
}

// ============================================================
// GRAPHICS
// ============================================================

Graphics::Graphics() 
    : window(nullptr), renderer(nullptr), 
      screenWidth(0), screenHeight(0),
      currentColor(Color::white()) {
}

Graphics::~Graphics() {
    shutdown();
}

bool Graphics::init(int width, int height, const std::string& title, 
                    bool fullscreen, bool vsync) {
    screenWidth = width;
    screenHeight = height;
    
    // Create window
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        windowFlags
    );
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (vsync) {
        rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
    }
    
    renderer = SDL_CreateRenderer(window, -1, rendererFlags);
    
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        window = nullptr;
        return false;
    }
    
    // Enable alpha blending by default
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Get actual window size (may differ in fullscreen)
    SDL_GetWindowSize(window, &screenWidth, &screenHeight);
    
    std::cout << "Omega Graphics initialized: " << screenWidth << "x" << screenHeight << std::endl;
    return true;
}

void Graphics::shutdown() {
    unloadAllTextures();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

// ========================================
// FRAME MANAGEMENT
// ========================================

void Graphics::clear(const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void Graphics::present() {
    SDL_RenderPresent(renderer);
}

// ========================================
// STATE
// ========================================

void Graphics::setColor(const Color& color) {
    currentColor = color;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

// ========================================
// PRIMITIVE DRAWING
// ========================================

void Graphics::drawPixel(int x, int y) {
    SDL_RenderDrawPoint(renderer, x, y);
}

void Graphics::drawPixel(int x, int y, const Color& color) {
    setColor(color);
    SDL_RenderDrawPoint(renderer, x, y);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2, const Color& color) {
    setColor(color);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Graphics::drawLine(const Vec2& a, const Vec2& b, const Color& color) {
    drawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
}

void Graphics::drawRect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &rect);
}

void Graphics::drawRect(int x, int y, int w, int h, const Color& color) {
    setColor(color);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &rect);
}

void Graphics::drawRect(const Rect& rect, const Color& color) {
    drawRect((int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h, color);
}

void Graphics::fillRect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void Graphics::fillRect(int x, int y, int w, int h, const Color& color) {
    setColor(color);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void Graphics::fillRect(const Rect& rect, const Color& color) {
    fillRect((int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h, color);
}

void Graphics::drawCircle(int cx, int cy, int radius, const Color& color) {
    setColor(color);
    
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        
        if (err <= 0) {
            y++;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void Graphics::fillCircle(int cx, int cy, int radius, const Color& color) {
    setColor(color);
    
    for (int y = -radius; y <= radius; y++) {
        int width = (int)std::sqrt(radius * radius - y * y);
        SDL_RenderDrawLine(renderer, cx - width, cy + y, cx + width, cy + y);
    }
}

void Graphics::fillQuad(int x1, int y1, int x2, int y2,
                        int x3, int y3, int x4, int y4, const Color& color) {
    setColor(color);
    
    // Find bounding box
    int minY = std::min({y1, y2, y3, y4});
    int maxY = std::max({y1, y2, y3, y4});
    
    // Simple scanline fill for convex quads
    // Points should be in order: top-left, top-right, bottom-right, bottom-left
    for (int y = minY; y <= maxY; y++) {
        // Find left and right edges at this scanline
        float t;
        int leftX, rightX;
        
        // Interpolate along left edge (1->4)
        if (y4 != y1) {
            t = (float)(y - y1) / (y4 - y1);
            leftX = (int)(x1 + t * (x4 - x1));
        } else {
            leftX = x1;
        }
        
        // Interpolate along right edge (2->3)
        if (y3 != y2) {
            t = (float)(y - y2) / (y3 - y2);
            rightX = (int)(x2 + t * (x3 - x2));
        } else {
            rightX = x2;
        }
        
        if (leftX > rightX) std::swap(leftX, rightX);
        
        SDL_RenderDrawLine(renderer, leftX, y, rightX, y);
    }
}

void Graphics::fillRectGradientH(int x, int y, int w, int h,
                                  const Color& left, const Color& right) {
    for (int dx = 0; dx < w; dx++) {
        float t = (float)dx / w;
        Color c = left.lerp(right, t);
        setColor(c);
        SDL_RenderDrawLine(renderer, x + dx, y, x + dx, y + h - 1);
    }
}

void Graphics::fillRectGradientV(int x, int y, int w, int h,
                                  const Color& top, const Color& bottom) {
    for (int dy = 0; dy < h; dy++) {
        float t = (float)dy / h;
        Color c = top.lerp(bottom, t);
        setColor(c);
        SDL_RenderDrawLine(renderer, x, y + dy, x + w - 1, y + dy);
    }
}

// ========================================
// TEXTURE MANAGEMENT
// ========================================

Texture* Graphics::loadTexture(const std::string& filename) {
    // Check cache first
    auto it = textureCache.find(filename);
    if (it != textureCache.end()) {
        return it->second.get();
    }
    
    // Load surface
    SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
    if (!surface) {
        std::cerr << "Failed to load texture: " << filename << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    // Create texture from surface
    SDL_Texture* sdlTex = SDL_CreateTextureFromSurface(renderer, surface);
    int w = surface->w;
    int h = surface->h;
    SDL_FreeSurface(surface);
    
    if (!sdlTex) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    // Enable blending
    SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
    
    // Create and cache texture
    auto texture = std::make_unique<Texture>();
    texture->sdlTexture = sdlTex;
    texture->width = w;
    texture->height = h;
    texture->name = filename;
    
    Texture* result = texture.get();
    textureCache[filename] = std::move(texture);
    
    return result;
}

Texture* Graphics::createTexture(int width, int height, const uint8_t* pixels) {
    SDL_Texture* sdlTex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        width, height
    );
    
    if (!sdlTex) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_UpdateTexture(sdlTex, nullptr, pixels, width * 4);
    SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
    
    // Generate unique name
    static int textureCounter = 0;
    std::string name = "_generated_" + std::to_string(textureCounter++);
    
    auto texture = std::make_unique<Texture>();
    texture->sdlTexture = sdlTex;
    texture->width = width;
    texture->height = height;
    texture->name = name;
    
    Texture* result = texture.get();
    textureCache[name] = std::move(texture);
    
    return result;
}

Texture* Graphics::createRenderTarget(int width, int height) {
    SDL_Texture* sdlTex = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET,
        width, height
    );
    
    if (!sdlTex) {
        std::cerr << "Failed to create render target: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
    
    static int rtCounter = 0;
    std::string name = "_rendertarget_" + std::to_string(rtCounter++);
    
    auto texture = std::make_unique<Texture>();
    texture->sdlTexture = sdlTex;
    texture->width = width;
    texture->height = height;
    texture->name = name;
    
    Texture* result = texture.get();
    textureCache[name] = std::move(texture);
    
    return result;
}

void Graphics::unloadTexture(const std::string& name) {
    textureCache.erase(name);
}

void Graphics::unloadAllTextures() {
    textureCache.clear();
}

// ========================================
// TEXTURE/SPRITE DRAWING
// ========================================

void Graphics::drawTexture(Texture* texture, int x, int y) {
    if (!texture || !texture->sdlTexture) return;
    
    SDL_Rect dst = {x, y, texture->width, texture->height};
    SDL_RenderCopy(renderer, texture->sdlTexture, nullptr, &dst);
}

void Graphics::drawTexture(Texture* texture, const DrawParams& params) {
    if (!texture || !texture->sdlTexture) return;
    
    int w = (int)(texture->width * params.scale.x);
    int h = (int)(texture->height * params.scale.y);
    
    SDL_Rect dst = {
        (int)params.position.x - w/2,
        (int)params.position.y - h/2,
        w, h
    };
    
    SDL_SetTextureColorMod(texture->sdlTexture, params.tint.r, params.tint.g, params.tint.b);
    SDL_SetTextureAlphaMod(texture->sdlTexture, params.tint.a);
    
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (params.flipH) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    if (params.flipV) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
    
    SDL_RenderCopyEx(renderer, texture->sdlTexture, nullptr, &dst,
                     params.rotation * RAD_TO_DEG, nullptr, flip);
    
    // Reset modulation
    SDL_SetTextureColorMod(texture->sdlTexture, 255, 255, 255);
    SDL_SetTextureAlphaMod(texture->sdlTexture, 255);
}

void Graphics::drawTexture(Texture* texture, const Recti& src, const Rect& dst) {
    if (!texture || !texture->sdlTexture) return;
    
    SDL_Rect srcRect = {src.x, src.y, src.w, src.h};
    SDL_Rect dstRect = {(int)dst.x, (int)dst.y, (int)dst.w, (int)dst.h};
    
    SDL_RenderCopy(renderer, texture->sdlTexture, &srcRect, &dstRect);
}

void Graphics::drawSprite(const Sprite& sprite, int x, int y) {
    if (!sprite.texture || !sprite.texture->sdlTexture) return;
    
    SDL_Rect srcRect = {sprite.srcRect.x, sprite.srcRect.y, 
                        sprite.srcRect.w, sprite.srcRect.h};
    SDL_Rect dstRect = {
        x - (int)(sprite.srcRect.w * sprite.origin.x),
        y - (int)(sprite.srcRect.h * sprite.origin.y),
        sprite.srcRect.w, sprite.srcRect.h
    };
    
    SDL_RenderCopy(renderer, sprite.texture->sdlTexture, &srcRect, &dstRect);
}

void Graphics::drawSprite(const Sprite& sprite, const DrawParams& params) {
    if (!sprite.texture || !sprite.texture->sdlTexture) return;
    
    int w = (int)(sprite.srcRect.w * params.scale.x);
    int h = (int)(sprite.srcRect.h * params.scale.y);
    
    SDL_Rect srcRect = {sprite.srcRect.x, sprite.srcRect.y,
                        sprite.srcRect.w, sprite.srcRect.h};
    SDL_Rect dstRect = {
        (int)params.position.x - (int)(w * sprite.origin.x),
        (int)params.position.y - (int)(h * sprite.origin.y),
        w, h
    };
    
    SDL_Point center = {(int)(w * sprite.origin.x), (int)(h * sprite.origin.y)};
    
    SDL_SetTextureColorMod(sprite.texture->sdlTexture, 
                           params.tint.r, params.tint.g, params.tint.b);
    SDL_SetTextureAlphaMod(sprite.texture->sdlTexture, params.tint.a);
    
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (params.flipH) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    if (params.flipV) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
    
    SDL_RenderCopyEx(renderer, sprite.texture->sdlTexture, &srcRect, &dstRect,
                     params.rotation * RAD_TO_DEG, &center, flip);
    
    SDL_SetTextureColorMod(sprite.texture->sdlTexture, 255, 255, 255);
    SDL_SetTextureAlphaMod(sprite.texture->sdlTexture, 255);
}

// ========================================
// RENDER TARGETS
// ========================================

void Graphics::setRenderTarget(Texture* target) {
    if (target && target->sdlTexture) {
        SDL_SetRenderTarget(renderer, target->sdlTexture);
    } else {
        SDL_SetRenderTarget(renderer, nullptr);
    }
}

} // namespace Omega
