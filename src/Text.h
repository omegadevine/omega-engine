#ifndef OMEGA_TEXT_H
#define OMEGA_TEXT_H

#include "Sprite.h"
#include "Shader.h"
#include "Texture.h"
#include <string>
#include <unordered_map>

// Character glyph information
struct Glyph {
    int x, y;           // Position in texture atlas
    int width, height;  // Size of glyph
    int xOffset, yOffset; // Offset when rendering
    int xAdvance;       // How much to advance cursor
};

// Bitmap font
class BitmapFont {
public:
    BitmapFont();
    ~BitmapFont() = default;
    
    bool loadFromFile(const std::string& fontFile, const std::string& textureFile);
    bool loadFromTexture(Texture* texture, int charWidth, int charHeight, const std::string& charset = "");
    
    void setTexture(Texture* texture) { m_texture = texture; }
    void setGlyph(char c, const Glyph& glyph);
    
    const Glyph* getGlyph(char c) const;
    Texture* getTexture() const { return m_texture; }
    int getLineHeight() const { return m_lineHeight; }
    
    // Calculate text dimensions
    void measureText(const std::string& text, int& width, int& height) const;

private:
    Texture* m_texture;
    std::unordered_map<char, Glyph> m_glyphs;
    int m_lineHeight;
};

// Text rendering
class Text {
public:
    Text();
    Text(const std::string& text, BitmapFont* font);
    ~Text() = default;
    
    void setText(const std::string& text) { m_text = text; }
    void setFont(BitmapFont* font) { m_font = font; }
    void setPosition(const Vector2& pos) { m_position = pos; }
    void setColor(const Color& color) { m_color = color; }
    void setScale(float scale) { m_scale = scale; }
    void setAlignment(int align) { m_alignment = align; } // 0=left, 1=center, 2=right
    
    std::string getText() const { return m_text; }
    Vector2 getPosition() const { return m_position; }
    Color getColor() const { return m_color; }
    float getScale() const { return m_scale; }
    
    void render(Shader* shader, int screenWidth, int screenHeight);
    
    // Utility
    void measureSize(int& width, int& height) const;

private:
    std::string m_text;
    BitmapFont* m_font;
    Vector2 m_position;
    Color m_color;
    float m_scale;
    int m_alignment;
};

// Font manager
class FontManager {
public:
    static FontManager& getInstance();
    
    BitmapFont* loadFont(const std::string& name, const std::string& fontFile, const std::string& textureFile);
    BitmapFont* loadMonospaceFont(const std::string& name, Texture* texture, int charWidth, int charHeight, const std::string& charset);
    BitmapFont* getFont(const std::string& name);
    void unloadFont(const std::string& name);
    void unloadAll();
    
    // Create default font (for debugging)
    BitmapFont* createDefaultFont(Texture* texture);

private:
    FontManager() = default;
    ~FontManager() { unloadAll(); }
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<BitmapFont>> m_fonts;
};

#endif // OMEGA_TEXT_H
