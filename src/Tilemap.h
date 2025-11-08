#ifndef OMEGA_TILEMAP_H
#define OMEGA_TILEMAP_H

#include "Sprite.h"
#include "Shader.h"
#include "Texture.h"
#include <vector>
#include <string>
#include <unordered_map>

// Tile data
struct Tile {
    int tileId;           // ID in the tileset
    bool solid;           // Collision flag
    int layer;            // Rendering layer
    
    Tile() : tileId(-1), solid(false), layer(0) {}
    Tile(int id, bool isSolid = false, int l = 0) 
        : tileId(id), solid(isSolid), layer(l) {}
};

// Tileset - texture atlas for tiles
class Tileset {
public:
    Tileset(Texture* texture, int tileWidth, int tileHeight);
    ~Tileset() = default;
    
    void setTileSize(int width, int height);
    void calculateGrid();
    
    // Get texture coordinates for a tile ID
    void getTileUV(int tileId, float& u0, float& v0, float& u1, float& v1) const;
    
    Texture* getTexture() const { return m_texture; }
    int getTileWidth() const { return m_tileWidth; }
    int getTileHeight() const { return m_tileHeight; }
    int getColumns() const { return m_columns; }
    int getRows() const { return m_rows; }
    int getTileCount() const { return m_columns * m_rows; }

private:
    Texture* m_texture;
    int m_tileWidth;
    int m_tileHeight;
    int m_columns;
    int m_rows;
};

// Tilemap - 2D grid of tiles
class Tilemap {
public:
    Tilemap(int width, int height, int tileWidth, int tileHeight);
    ~Tilemap() = default;
    
    // Tile access
    void setTile(int x, int y, const Tile& tile);
    Tile getTile(int x, int y) const;
    void clearTile(int x, int y);
    
    // Tileset
    void setTileset(Tileset* tileset) { m_tileset = tileset; }
    Tileset* getTileset() const { return m_tileset; }
    
    // Rendering
    void render(Shader* shader, int screenWidth, int screenHeight, const Vector2& cameraPos = Vector2(0, 0));
    void renderLayer(int layer, Shader* shader, int screenWidth, int screenHeight, const Vector2& cameraPos = Vector2(0, 0));
    
    // Collision
    bool isTileSolid(int x, int y) const;
    bool worldToTile(float worldX, float worldY, int& tileX, int& tileY) const;
    void tileToWorld(int tileX, int tileY, float& worldX, float& worldY) const;
    
    // Properties
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileWidth() const { return m_tileWidth; }
    int getTileHeight() const { return m_tileHeight; }
    
    // File I/O
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    
    // Fill patterns
    void fill(const Tile& tile);
    void fillRect(int x, int y, int width, int height, const Tile& tile);
    void fillLayer(int layer, const Tile& tile);

private:
    int coordToIndex(int x, int y) const;
    bool isValidCoord(int x, int y) const;
    
    int m_width;
    int m_height;
    int m_tileWidth;
    int m_tileHeight;
    std::vector<Tile> m_tiles;
    Tileset* m_tileset;
};

// Tilemap manager for multiple layers
class TilemapManager {
public:
    TilemapManager();
    ~TilemapManager() = default;
    
    Tilemap* addLayer(const std::string& name, int width, int height, int tileWidth, int tileHeight);
    Tilemap* getLayer(const std::string& name);
    void removeLayer(const std::string& name);
    void clear();
    
    void renderAll(Shader* shader, int screenWidth, int screenHeight, const Vector2& cameraPos = Vector2(0, 0));
    
    size_t getLayerCount() const { return m_layers.size(); }

private:
    std::unordered_map<std::string, std::unique_ptr<Tilemap>> m_layers;
    std::vector<std::string> m_layerOrder; // For rendering order
};

#endif // OMEGA_TILEMAP_H
