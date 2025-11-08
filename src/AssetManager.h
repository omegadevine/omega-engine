#ifndef OMEGA_ASSET_MANAGER_H
#define OMEGA_ASSET_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "Texture.h"
#include "Shader.h"

class AssetManager {
public:
    static AssetManager& getInstance();

    // Texture management
    Texture* loadTexture(const std::string& name, const std::string& filepath);
    Texture* getTexture(const std::string& name);
    bool hasTexture(const std::string& name) const;
    void unloadTexture(const std::string& name);

    // Shader management
    Shader* loadShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
    Shader* getShader(const std::string& name);
    bool hasShader(const std::string& name) const;
    void unloadShader(const std::string& name);

    // Utility
    void unloadAll();
    void unloadAllTextures();
    void unloadAllShaders();
    
    size_t getTextureCount() const { return m_textures.size(); }
    size_t getShaderCount() const { return m_shaders.size(); }

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};

#endif // OMEGA_ASSET_MANAGER_H
