#include "Text.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// ============================================================================
// BitmapFont Implementation
// ============================================================================

BitmapFont::BitmapFont()
    : m_texture(nullptr)
    , m_lineHeight(16) {
}

bool BitmapFont::loadFromFile(const std::string& fontFile, const std::string& textureFile) {
    // Simple BMFont .fnt file parser
    std::ifstream file(fontFile);
    if (!file.is_open()) {
        std::cerr << "BitmapFont: Failed to open font file: " << fontFile << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        
        if (type == "common") {
            std::string token;
            while (iss >> token) {
                if (token.find("lineHeight=") == 0) {
                    m_lineHeight = std::stoi(token.substr(11));
                }
            }
        }
        else if (type == "char") {
            Glyph glyph;
            char c = 0;
            std::string token;
            
            while (iss >> token) {
                if (token.find("id=") == 0) {
                    c = static_cast<char>(std::stoi(token.substr(3)));
                }
                else if (token.find("x=") == 0) {
                    glyph.x = std::stoi(token.substr(2));
                }
                else if (token.find("y=") == 0) {
                    glyph.y = std::stoi(token.substr(2));
                }
                else if (token.find("width=") == 0) {
                    glyph.width = std::stoi(token.substr(6));
                }
                else if (token.find("height=") == 0) {
                    glyph.height = std::stoi(token.substr(7));
                }
                else if (token.find("xoffset=") == 0) {
                    glyph.xOffset = std::stoi(token.substr(8));
                }
                else if (token.find("yoffset=") == 0) {
                    glyph.yOffset = std::stoi(token.substr(8));
                }
                else if (token.find("xadvance=") == 0) {
                    glyph.xAdvance = std::stoi(token.substr(9));
                }
            }
            
            m_glyphs[c] = glyph;
        }
    }
    
    file.close();
    std::cout << "BitmapFont: Loaded " << m_glyphs.size() << " glyphs from " << fontFile << std::endl;
    return true;
}

bool BitmapFont::loadFromTexture(Texture* texture, int charWidth, int charHeight, const std::string& charset) {
    m_texture = texture;
    m_lineHeight = charHeight;
    
    // Default ASCII charset if none provided
    std::string chars = charset.empty() ? 
        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" : 
        charset;
    
    int charsPerRow = texture->getWidth() / charWidth;
    
    for (size_t i = 0; i < chars.length(); i++) {
        char c = chars[i];
        Glyph glyph;
        
        int row = i / charsPerRow;
        int col = i % charsPerRow;
        
        glyph.x = col * charWidth;
        glyph.y = row * charHeight;
        glyph.width = charWidth;
        glyph.height = charHeight;
        glyph.xOffset = 0;
        glyph.yOffset = 0;
        glyph.xAdvance = charWidth;
        
        m_glyphs[c] = glyph;
    }
    
    std::cout << "BitmapFont: Created monospace font with " << m_glyphs.size() << " characters" << std::endl;
    return true;
}

void BitmapFont::setGlyph(char c, const Glyph& glyph) {
    m_glyphs[c] = glyph;
}

const Glyph* BitmapFont::getGlyph(char c) const {
    auto it = m_glyphs.find(c);
    if (it != m_glyphs.end()) {
        return &it->second;
    }
    return nullptr;
}

void BitmapFont::measureText(const std::string& text, int& width, int& height) const {
    width = 0;
    height = m_lineHeight;
    
    int currentLineWidth = 0;
    int lineCount = 1;
    
    for (char c : text) {
        if (c == '\n') {
            width = std::max(width, currentLineWidth);
            currentLineWidth = 0;
            lineCount++;
            continue;
        }
        
        const Glyph* glyph = getGlyph(c);
        if (glyph) {
            currentLineWidth += glyph->xAdvance;
        }
    }
    
    width = std::max(width, currentLineWidth);
    height = lineCount * m_lineHeight;
}

// ============================================================================
// Text Implementation
// ============================================================================

Text::Text()
    : m_text("")
    , m_font(nullptr)
    , m_position(0, 0)
    , m_color(1, 1, 1, 1)
    , m_scale(1.0f)
    , m_alignment(0) {
}

Text::Text(const std::string& text, BitmapFont* font)
    : m_text(text)
    , m_font(font)
    , m_position(0, 0)
    , m_color(1, 1, 1, 1)
    , m_scale(1.0f)
    , m_alignment(0) {
}

void Text::render(Shader* shader, int screenWidth, int screenHeight) {
    if (!m_font || !m_font->getTexture() || m_text.empty()) return;
    
    Sprite charSprite;
    charSprite.setTexture(m_font->getTexture());
    charSprite.setColor(m_color);
    
    Vector2 cursor = m_position;
    
    // Calculate alignment offset
    if (m_alignment > 0) {
        int textWidth, textHeight;
        measureSize(textWidth, textHeight);
        
        if (m_alignment == 1) { // Center
            cursor.x -= textWidth * 0.5f;
        } else if (m_alignment == 2) { // Right
            cursor.x -= textWidth;
        }
    }
    
    for (char c : m_text) {
        if (c == '\n') {
            cursor.x = m_position.x;
            cursor.y += m_font->getLineHeight() * m_scale;
            continue;
        }
        
        const Glyph* glyph = m_font->getGlyph(c);
        if (!glyph) continue;
        
        // Set sprite properties
        Vector2 pos;
        pos.x = cursor.x + glyph->xOffset * m_scale;
        pos.y = cursor.y + glyph->yOffset * m_scale;
        
        charSprite.setPosition(pos);
        charSprite.setSize(Vector2(glyph->width * m_scale, glyph->height * m_scale));
        
        // Set texture coordinates (UV)
        float texWidth = m_font->getTexture()->getWidth();
        float texHeight = m_font->getTexture()->getHeight();
        
        // Note: This is a simplified version. For proper UV support,
        // you'd need to extend the Sprite class to support custom UVs
        
        charSprite.draw(shader, screenWidth, screenHeight);
        
        cursor.x += glyph->xAdvance * m_scale;
    }
}

void Text::measureSize(int& width, int& height) const {
    if (m_font) {
        m_font->measureText(m_text, width, height);
        width = static_cast<int>(width * m_scale);
        height = static_cast<int>(height * m_scale);
    } else {
        width = height = 0;
    }
}

// ============================================================================
// FontManager Implementation
// ============================================================================

FontManager& FontManager::getInstance() {
    static FontManager instance;
    return instance;
}

BitmapFont* FontManager::loadFont(const std::string& name, const std::string& fontFile, const std::string& textureFile) {
    auto font = std::make_unique<BitmapFont>();
    if (!font->loadFromFile(fontFile, textureFile)) {
        return nullptr;
    }
    
    BitmapFont* ptr = font.get();
    m_fonts[name] = std::move(font);
    return ptr;
}

BitmapFont* FontManager::loadMonospaceFont(const std::string& name, Texture* texture, int charWidth, int charHeight, const std::string& charset) {
    auto font = std::make_unique<BitmapFont>();
    if (!font->loadFromTexture(texture, charWidth, charHeight, charset)) {
        return nullptr;
    }
    
    BitmapFont* ptr = font.get();
    m_fonts[name] = std::move(font);
    return ptr;
}

BitmapFont* FontManager::getFont(const std::string& name) {
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
        return it->second.get();
    }
    return nullptr;
}

void FontManager::unloadFont(const std::string& name) {
    m_fonts.erase(name);
}

void FontManager::unloadAll() {
    m_fonts.clear();
}

BitmapFont* FontManager::createDefaultFont(Texture* texture) {
    // Create a simple 8x8 monospace font
    auto font = std::make_unique<BitmapFont>();
    font->loadFromTexture(texture, 8, 8);
    
    BitmapFont* ptr = font.get();
    m_fonts["default"] = std::move(font);
    return ptr;
}
