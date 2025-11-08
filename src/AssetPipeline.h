#ifndef OMEGA_ASSET_PIPELINE_H
#define OMEGA_ASSET_PIPELINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

// Asset metadata
struct AssetMetadata {
    std::string name;
    std::string type;
    std::string sourcePath;
    std::string outputPath;
    size_t fileSize;
    std::string checksum;
    long long lastModified;
    std::unordered_map<std::string, std::string> customData;
};

// Asset processor interface
class IAssetProcessor {
public:
    virtual ~IAssetProcessor() = default;
    virtual bool process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) = 0;
    virtual std::string getInputExtension() const = 0;
    virtual std::string getOutputExtension() const = 0;
};

// Texture processor
class TextureProcessor : public IAssetProcessor {
public:
    bool process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) override;
    std::string getInputExtension() const override { return ".png"; }
    std::string getOutputExtension() const override { return ".otex"; } // Omega texture
    
    void setCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }
    void setMipmapEnabled(bool enabled) { m_mipmapEnabled = enabled; }

private:
    bool m_compressionEnabled = false;
    bool m_mipmapEnabled = false;
};

// Audio processor
class AudioProcessor : public IAssetProcessor {
public:
    bool process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) override;
    std::string getInputExtension() const override { return ".wav"; }
    std::string getOutputExtension() const override { return ".oaudio"; } // Omega audio
    
    void setCompressionQuality(float quality) { m_quality = quality; }

private:
    float m_quality = 0.8f;
};

// Font processor
class FontProcessor : public IAssetProcessor {
public:
    bool process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) override;
    std::string getInputExtension() const override { return ".ttf"; }
    std::string getOutputExtension() const override { return ".ofnt"; } // Omega font
    
    void setFontSize(int size) { m_fontSize = size; }
    void setCharset(const std::string& charset) { m_charset = charset; }

private:
    int m_fontSize = 16;
    std::string m_charset = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
};

// Asset pipeline - processes and manages assets
class AssetPipeline {
public:
    static AssetPipeline& getInstance();
    
    // Processor management
    void registerProcessor(const std::string& extension, IAssetProcessor* processor);
    IAssetProcessor* getProcessor(const std::string& extension);
    
    // Asset processing
    bool processAsset(const std::string& inputPath, const std::string& outputPath = "");
    bool processDirectory(const std::string& inputDir, const std::string& outputDir, bool recursive = true);
    
    // Metadata management
    bool loadMetadata(const std::string& metadataFile);
    bool saveMetadata(const std::string& metadataFile) const;
    AssetMetadata* getMetadata(const std::string& assetName);
    
    // Asset validation
    bool validateAsset(const std::string& assetPath);
    bool needsReprocessing(const std::string& assetPath) const;
    
    // Utility
    std::string calculateChecksum(const std::string& filePath) const;
    long long getFileModifiedTime(const std::string& filePath) const;
    size_t getFileSize(const std::string& filePath) const;
    
    void clear();
    size_t getAssetCount() const { return m_metadata.size(); }
    
    // Callbacks
    using ProgressCallback = std::function<void(const std::string&, float)>;
    void setProgressCallback(ProgressCallback callback) { m_progressCallback = callback; }

private:
    AssetPipeline();
    ~AssetPipeline();
    AssetPipeline(const AssetPipeline&) = delete;
    AssetPipeline& operator=(const AssetPipeline&) = delete;
    
    std::unordered_map<std::string, IAssetProcessor*> m_processors;
    std::unordered_map<std::string, AssetMetadata> m_metadata;
    ProgressCallback m_progressCallback;
};

// Asset hot reloading
class AssetHotReloader {
public:
    static AssetHotReloader& getInstance();
    
    void watchDirectory(const std::string& directory);
    void unwatchDirectory(const std::string& directory);
    
    void update(); // Check for file changes
    
    using ReloadCallback = std::function<void(const std::string&)>;
    void registerCallback(const std::string& extension, ReloadCallback callback);
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    AssetHotReloader();
    ~AssetHotReloader() = default;
    AssetHotReloader(const AssetHotReloader&) = delete;
    AssetHotReloader& operator=(const AssetHotReloader&) = delete;
    
    struct WatchedFile {
        std::string path;
        long long lastModified;
    };
    
    std::vector<std::string> m_watchedDirectories;
    std::unordered_map<std::string, WatchedFile> m_watchedFiles;
    std::unordered_map<std::string, ReloadCallback> m_callbacks;
    bool m_enabled;
};

// Asset bundler - packages assets for distribution
class AssetBundler {
public:
    static AssetBundler& getInstance();
    
    bool createBundle(const std::string& bundleName, const std::vector<std::string>& assetPaths);
    bool extractBundle(const std::string& bundlePath, const std::string& outputDir);
    
    bool addToBundle(const std::string& bundlePath, const std::string& assetPath);
    std::vector<std::string> listBundleContents(const std::string& bundlePath);
    
    void setCompression(bool enabled) { m_compressionEnabled = enabled; }
    void setEncryption(bool enabled) { m_encryptionEnabled = enabled; }

private:
    AssetBundler();
    ~AssetBundler() = default;
    AssetBundler(const AssetBundler&) = delete;
    AssetBundler& operator=(const AssetBundler&) = delete;
    
    bool m_compressionEnabled;
    bool m_encryptionEnabled;
};

#endif // OMEGA_ASSET_PIPELINE_H
