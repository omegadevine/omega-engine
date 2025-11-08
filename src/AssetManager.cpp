#include "AssetManager.h"
#include <iostream>

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

// Texture management
Texture* AssetManager::loadTexture(const std::string& name, const std::string& filepath) {
    // Check if already loaded
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        std::cout << "Texture '" << name << "' already loaded, returning cached version" << std::endl;
        return it->second.get();
    }

    // Create and load new texture
    auto texture = std::make_shared<Texture>();
    if (!texture->loadFromFile(filepath)) {
        std::cerr << "AssetManager: Failed to load texture '" << name << "' from " << filepath << std::endl;
        return nullptr;
    }

    // Store and return
    m_textures[name] = texture;
    std::cout << "AssetManager: Loaded texture '" << name << "' (" << texture->getWidth() << "x" << texture->getHeight() << ")" << std::endl;
    return texture.get();
}

Texture* AssetManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second.get();
    }
    
    std::cerr << "AssetManager: Texture '" << name << "' not found" << std::endl;
    return nullptr;
}

bool AssetManager::hasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

void AssetManager::unloadTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        std::cout << "AssetManager: Unloaded texture '" << name << "'" << std::endl;
        m_textures.erase(it);
    }
}

// Shader management
Shader* AssetManager::loadShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
    // Check if already loaded
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        std::cout << "Shader '" << name << "' already loaded, returning cached version" << std::endl;
        return it->second.get();
    }

    // Create and compile new shader
    auto shader = std::make_shared<Shader>();
    if (!shader->loadFromSource(vertexSrc, fragmentSrc)) {
        std::cerr << "AssetManager: Failed to load shader '" << name << "'" << std::endl;
        return nullptr;
    }

    // Store and return
    m_shaders[name] = shader;
    std::cout << "AssetManager: Loaded shader '" << name << "' (ID: " << shader->getProgramID() << ")" << std::endl;
    return shader.get();
}

Shader* AssetManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.get();
    }
    
    std::cerr << "AssetManager: Shader '" << name << "' not found" << std::endl;
    return nullptr;
}

bool AssetManager::hasShader(const std::string& name) const {
    return m_shaders.find(name) != m_shaders.end();
}

void AssetManager::unloadShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        std::cout << "AssetManager: Unloaded shader '" << name << "'" << std::endl;
        m_shaders.erase(it);
    }
}

// Utility
void AssetManager::unloadAll() {
    std::cout << "AssetManager: Unloading all assets..." << std::endl;
    unloadAllTextures();
    unloadAllShaders();
}

void AssetManager::unloadAllTextures() {
    std::cout << "AssetManager: Unloading " << m_textures.size() << " texture(s)" << std::endl;
    m_textures.clear();
}

void AssetManager::unloadAllShaders() {
    std::cout << "AssetManager: Unloading " << m_shaders.size() << " shader(s)" << std::endl;
    m_shaders.clear();
}
