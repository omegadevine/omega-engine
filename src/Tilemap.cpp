#include "Tilemap.h"
#include <fstream>
#include <iostream>
#include <cmath>

// ============================================================================
// Tileset Implementation
// ============================================================================

Tileset::Tileset(Texture* texture, int tileWidth, int tileHeight)
    : m_texture(texture)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight)
    , m_columns(0)
    , m_rows(0) {
    
    calculateGrid();
}

void Tileset::setTileSize(int width, int height) {
    m_tileWidth = width;
    m_tileHeight = height;
    calculateGrid();
}

void Tileset::calculateGrid() {
    if (m_texture) {
        m_columns = m_texture->getWidth() / m_tileWidth;
        m_rows = m_texture->getHeight() / m_tileHeight;
    }
}

void Tileset::getTileUV(int tileId, float& u0, float& v0, float& u1, float& v1) const {
    if (!m_texture || tileId < 0) {
        u0 = v0 = 0.0f;
        u1 = v1 = 1.0f;
        return;
    }
    
    int column = tileId % m_columns;
    int row = tileId / m_columns;
    
    float texWidth = static_cast<float>(m_texture->getWidth());
    float texHeight = static_cast<float>(m_texture->getHeight());
    
    u0 = (column * m_tileWidth) / texWidth;
    v0 = (row * m_tileHeight) / texHeight;
    u1 = ((column + 1) * m_tileWidth) / texWidth;
    v1 = ((row + 1) * m_tileHeight) / texHeight;
}

// ============================================================================
// Tilemap Implementation
// ============================================================================

Tilemap::Tilemap(int width, int height, int tileWidth, int tileHeight)
    : m_width(width)
    , m_height(height)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight)
    , m_tileset(nullptr) {
    
    m_tiles.resize(width * height);
}

int Tilemap::coordToIndex(int x, int y) const {
    return y * m_width + x;
}

bool Tilemap::isValidCoord(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

void Tilemap::setTile(int x, int y, const Tile& tile) {
    if (isValidCoord(x, y)) {
        m_tiles[coordToIndex(x, y)] = tile;
    }
}

Tile Tilemap::getTile(int x, int y) const {
    if (isValidCoord(x, y)) {
        return m_tiles[coordToIndex(x, y)];
    }
    return Tile();
}

void Tilemap::clearTile(int x, int y) {
    setTile(x, y, Tile());
}

bool Tilemap::isTileSolid(int x, int y) const {
    if (isValidCoord(x, y)) {
        return m_tiles[coordToIndex(x, y)].solid;
    }
    return false;
}

bool Tilemap::worldToTile(float worldX, float worldY, int& tileX, int& tileY) const {
    tileX = static_cast<int>(std::floor(worldX / m_tileWidth));
    tileY = static_cast<int>(std::floor(worldY / m_tileHeight));
    return isValidCoord(tileX, tileY);
}

void Tilemap::tileToWorld(int tileX, int tileY, float& worldX, float& worldY) const {
    worldX = tileX * m_tileWidth;
    worldY = tileY * m_tileHeight;
}

void Tilemap::render(Shader* shader, int screenWidth, int screenHeight, const Vector2& cameraPos) {
    if (!m_tileset || !m_tileset->getTexture()) return;
    
    Sprite tileSprite;
    tileSprite.setTexture(m_tileset->getTexture());
    tileSprite.setSize(Vector2(m_tileWidth, m_tileHeight));
    
    // Calculate visible tile range (basic culling)
    int startX = std::max(0, static_cast<int>(cameraPos.x / m_tileWidth) - 1);
    int startY = std::max(0, static_cast<int>(cameraPos.y / m_tileHeight) - 1);
    int endX = std::min(m_width, static_cast<int>((cameraPos.x + screenWidth) / m_tileWidth) + 2);
    int endY = std::min(m_height, static_cast<int>((cameraPos.y + screenHeight) / m_tileHeight) + 2);
    
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            const Tile& tile = m_tiles[coordToIndex(x, y)];
            if (tile.tileId < 0) continue; // Empty tile
            
            // Get UV coordinates for this tile
            float u0, v0, u1, v1;
            m_tileset->getTileUV(tile.tileId, u0, v0, u1, v1);
            
            // Set sprite position
            float worldX, worldY;
            tileToWorld(x, y, worldX, worldY);
            tileSprite.setPosition(Vector2(worldX, worldY));
            
            // Render tile
            tileSprite.draw(shader, screenWidth, screenHeight);
        }
    }
}

void Tilemap::renderLayer(int layer, Shader* shader, int screenWidth, int screenHeight, const Vector2& cameraPos) {
    if (!m_tileset || !m_tileset->getTexture()) return;
    
    Sprite tileSprite;
    tileSprite.setTexture(m_tileset->getTexture());
    tileSprite.setSize(Vector2(m_tileWidth, m_tileHeight));
    
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            const Tile& tile = m_tiles[coordToIndex(x, y)];
            if (tile.tileId < 0 || tile.layer != layer) continue;
            
            float u0, v0, u1, v1;
            m_tileset->getTileUV(tile.tileId, u0, v0, u1, v1);
            
            float worldX, worldY;
            tileToWorld(x, y, worldX, worldY);
            tileSprite.setPosition(Vector2(worldX, worldY));
            
            tileSprite.draw(shader, screenWidth, screenHeight);
        }
    }
}

void Tilemap::fill(const Tile& tile) {
    std::fill(m_tiles.begin(), m_tiles.end(), tile);
}

void Tilemap::fillRect(int x, int y, int width, int height, const Tile& tile) {
    for (int ty = y; ty < y + height && ty < m_height; ty++) {
        for (int tx = x; tx < x + width && tx < m_width; tx++) {
            setTile(tx, ty, tile);
        }
    }
}

void Tilemap::fillLayer(int layer, const Tile& tile) {
    for (auto& t : m_tiles) {
        if (t.layer == layer) {
            t = tile;
        }
    }
}

bool Tilemap::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Tilemap: Failed to open file: " << filename << std::endl;
        return false;
    }
    
    // Read header
    file.read(reinterpret_cast<char*>(&m_width), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_height), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_tileWidth), sizeof(int));
    file.read(reinterpret_cast<char*>(&m_tileHeight), sizeof(int));
    
    // Read tiles
    m_tiles.resize(m_width * m_height);
    file.read(reinterpret_cast<char*>(m_tiles.data()), m_tiles.size() * sizeof(Tile));
    
    file.close();
    std::cout << "Tilemap: Loaded from " << filename << std::endl;
    return true;
}

bool Tilemap::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Tilemap: Failed to create file: " << filename << std::endl;
        return false;
    }
    
    // Write header
    file.write(reinterpret_cast<const char*>(&m_width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&m_height), sizeof(int));
    file.write(reinterpret_cast<const char*>(&m_tileWidth), sizeof(int));
    file.write(reinterpret_cast<const char*>(&m_tileHeight), sizeof(int));
    
    // Write tiles
    file.write(reinterpret_cast<const char*>(m_tiles.data()), m_tiles.size() * sizeof(Tile));
    
    file.close();
    std::cout << "Tilemap: Saved to " << filename << std::endl;
    return true;
}

// ============================================================================
// TilemapManager Implementation
// ============================================================================

TilemapManager::TilemapManager() {
}

Tilemap* TilemapManager::addLayer(const std::string& name, int width, int height, int tileWidth, int tileHeight) {
    auto tilemap = std::make_unique<Tilemap>(width, height, tileWidth, tileHeight);
    Tilemap* ptr = tilemap.get();
    m_layers[name] = std::move(tilemap);
    m_layerOrder.push_back(name);
    return ptr;
}

Tilemap* TilemapManager::getLayer(const std::string& name) {
    auto it = m_layers.find(name);
    if (it != m_layers.end()) {
        return it->second.get();
    }
    return nullptr;
}

void TilemapManager::removeLayer(const std::string& name) {
    m_layers.erase(name);
    m_layerOrder.erase(
        std::remove(m_layerOrder.begin(), m_layerOrder.end(), name),
        m_layerOrder.end()
    );
}

void TilemapManager::clear() {
    m_layers.clear();
    m_layerOrder.clear();
}

void TilemapManager::renderAll(Shader* shader, int screenWidth, int screenHeight, const Vector2& cameraPos) {
    // Render in layer order
    for (const auto& layerName : m_layerOrder) {
        auto it = m_layers.find(layerName);
        if (it != m_layers.end()) {
            it->second->render(shader, screenWidth, screenHeight, cameraPos);
        }
    }
}
