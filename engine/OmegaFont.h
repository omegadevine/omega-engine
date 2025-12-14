/**
 * OMEGA ENGINE - Bitmap Font System
 * 
 * Simple built-in bitmap font for text rendering without SDL_ttf dependency.
 * Uses a procedurally generated 8x8 pixel font.
 */

#ifndef OMEGA_FONT_H
#define OMEGA_FONT_H

#include "OmegaGraphics.h"
#include <string>

namespace Omega {

/**
 * Built-in bitmap font for basic text rendering
 */
class BitmapFont {
private:
    Graphics* gfx;
    int charWidth;
    int charHeight;
    int spacing;
    
    // Draw a single character using primitives
    void drawChar(int x, int y, char c, const Color& color, int scale);
    
public:
    BitmapFont(Graphics* graphics);
    
    /**
     * Draw text at position
     */
    void drawText(int x, int y, const std::string& text, const Color& color, int scale = 1);
    
    /**
     * Draw text centered horizontally
     */
    void drawTextCentered(int y, const std::string& text, const Color& color, int screenWidth, int scale = 1);
    
    /**
     * Draw text right-aligned
     */
    void drawTextRight(int x, int y, const std::string& text, const Color& color, int scale = 1);
    
    /**
     * Get width of text in pixels
     */
    int getTextWidth(const std::string& text, int scale = 1) const;
    
    /**
     * Get height of text in pixels
     */
    int getTextHeight(int scale = 1) const;
    
    /**
     * Set character spacing
     */
    void setSpacing(int s) { spacing = s; }
};

} // namespace Omega

#endif // OMEGA_FONT_H
