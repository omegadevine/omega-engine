#include "AssetPipeline.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>

namespace fs = std::filesystem;

// ============================================================================
// TextureProcessor Implementation
// ============================================================================

bool TextureProcessor::process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) {
    std::cout << "Processing texture: " << inputPath << std::endl;
    
    // In a real implementation, you would:
    // 1. Load the image using stb_image or similar
    // 2. Apply compression if enabled
    // 3. Generate mipmaps if enabled
    // 4. Save to custom format
    
    // For now, just copy the file
    try {
        fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
        
        metadata.type = "texture";
        metadata.sourcePath = inputPath;
        metadata.outputPath = outputPath;
        metadata.customData["compression"] = m_compressionEnabled ? "true" : "false";
        metadata.customData["mipmaps"] = m_mipmapEnabled ? "true" : "false";
        
        std::cout << "Texture processed: " << outputPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to process texture: " << e.what() << std::endl;
        return false;
    }
}

// ============================================================================
// AudioProcessor Implementation
// ============================================================================

bool AudioProcessor::process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) {
    std::cout << "Processing audio: " << inputPath << std::endl;
    
    // In a real implementation, you would:
    // 1. Load the audio file
    // 2. Apply compression (OGG Vorbis, etc.)
    // 3. Normalize volume
    // 4. Save to custom format
    
    try {
        fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
        
        metadata.type = "audio";
        metadata.sourcePath = inputPath;
        metadata.outputPath = outputPath;
        metadata.customData["quality"] = std::to_string(m_quality);
        
        std::cout << "Audio processed: " << outputPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to process audio: " << e.what() << std::endl;
        return false;
    }
}

// ============================================================================
// FontProcessor Implementation
// ============================================================================

bool FontProcessor::process(const std::string& inputPath, const std::string& outputPath, AssetMetadata& metadata) {
    std::cout << "Processing font: " << inputPath << std::endl;
    
    // In a real implementation, you would:
    // 1. Load TTF using FreeType
    // 2. Rasterize glyphs for the charset
    // 3. Pack into texture atlas
    // 4. Generate .fnt file with glyph data
    
    try {
        fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
        
        metadata.type = "font";
        metadata.sourcePath = inputPath;
        metadata.outputPath = outputPath;
        metadata.customData["fontSize"] = std::to_string(m_fontSize);
        metadata.customData["charset"] = m_charset;
        
        std::cout << "Font processed: " << outputPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to process font: " << e.what() << std::endl;
        return false;
    }
}

// ============================================================================
// AssetPipeline Implementation
// ============================================================================

AssetPipeline::AssetPipeline()
    : m_progressCallback(nullptr) {
}

AssetPipeline::~AssetPipeline() {
    // Clean up processors
    for (auto& pair : m_processors) {
        delete pair.second;
    }
}

AssetPipeline& AssetPipeline::getInstance() {
    static AssetPipeline instance;
    return instance;
}

void AssetPipeline::registerProcessor(const std::string& extension, IAssetProcessor* processor) {
    m_processors[extension] = processor;
    std::cout << "Registered processor for: " << extension << std::endl;
}

IAssetProcessor* AssetPipeline::getProcessor(const std::string& extension) {
    auto it = m_processors.find(extension);
    return (it != m_processors.end()) ? it->second : nullptr;
}

bool AssetPipeline::processAsset(const std::string& inputPath, const std::string& outputPath) {
    if (!fs::exists(inputPath)) {
        std::cerr << "Asset not found: " << inputPath << std::endl;
        return false;
    }
    
    std::string extension = fs::path(inputPath).extension().string();
    IAssetProcessor* processor = getProcessor(extension);
    
    if (!processor) {
        std::cerr << "No processor found for extension: " << extension << std::endl;
        return false;
    }
    
    std::string output = outputPath.empty() ? 
        (fs::path(inputPath).stem().string() + processor->getOutputExtension()) : 
        outputPath;
    
    AssetMetadata metadata;
    metadata.name = fs::path(inputPath).stem().string();
    metadata.fileSize = getFileSize(inputPath);
    metadata.checksum = calculateChecksum(inputPath);
    metadata.lastModified = getFileModifiedTime(inputPath);
    
    if (processor->process(inputPath, output, metadata)) {
        m_metadata[metadata.name] = metadata;
        return true;
    }
    
    return false;
}

bool AssetPipeline::processDirectory(const std::string& inputDir, const std::string& outputDir, bool recursive) {
    if (!fs::exists(inputDir)) {
        std::cerr << "Directory not found: " << inputDir << std::endl;
        return false;
    }
    
    // Create output directory
    fs::create_directories(outputDir);
    
    int processedCount = 0;
    int totalFiles = 0;
    
    // Count files
    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
            if (entry.is_regular_file()) totalFiles++;
        }
    } else {
        for (const auto& entry : fs::directory_iterator(inputDir)) {
            if (entry.is_regular_file()) totalFiles++;
        }
    }
    
    // Process files
    auto processFile = [&](const fs::directory_entry& entry) {
        if (entry.is_regular_file()) {
            std::string inputPath = entry.path().string();
            std::string extension = entry.path().extension().string();
            IAssetProcessor* processor = getProcessor(extension);
            
            if (processor) {
                std::string outputPath = outputDir + "/" + 
                    entry.path().stem().string() + 
                    processor->getOutputExtension();
                
                if (processAsset(inputPath, outputPath)) {
                    processedCount++;
                    
                    if (m_progressCallback) {
                        float progress = static_cast<float>(processedCount) / totalFiles;
                        m_progressCallback(inputPath, progress);
                    }
                }
            }
        }
    };
    
    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
            processFile(entry);
        }
    } else {
        for (const auto& entry : fs::directory_iterator(inputDir)) {
            processFile(entry);
        }
    }
    
    std::cout << "Processed " << processedCount << " of " << totalFiles << " files" << std::endl;
    return processedCount > 0;
}

bool AssetPipeline::loadMetadata(const std::string& metadataFile) {
    std::ifstream file(metadataFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open metadata file: " << metadataFile << std::endl;
        return false;
    }
    
    // Simple CSV-like format (in production, use JSON)
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        AssetMetadata metadata;
        
        // Parse line (simplified)
        std::getline(iss, metadata.name, ',');
        std::getline(iss, metadata.type, ',');
        std::getline(iss, metadata.sourcePath, ',');
        std::getline(iss, metadata.outputPath, ',');
        
        m_metadata[metadata.name] = metadata;
    }
    
    file.close();
    std::cout << "Loaded metadata for " << m_metadata.size() << " assets" << std::endl;
    return true;
}

bool AssetPipeline::saveMetadata(const std::string& metadataFile) const {
    std::ofstream file(metadataFile);
    if (!file.is_open()) {
        std::cerr << "Failed to create metadata file: " << metadataFile << std::endl;
        return false;
    }
    
    for (const auto& pair : m_metadata) {
        const AssetMetadata& meta = pair.second;
        file << meta.name << "," 
             << meta.type << "," 
             << meta.sourcePath << "," 
             << meta.outputPath << "\n";
    }
    
    file.close();
    std::cout << "Saved metadata for " << m_metadata.size() << " assets" << std::endl;
    return true;
}

AssetMetadata* AssetPipeline::getMetadata(const std::string& assetName) {
    auto it = m_metadata.find(assetName);
    return (it != m_metadata.end()) ? &it->second : nullptr;
}

bool AssetPipeline::validateAsset(const std::string& assetPath) {
    if (!fs::exists(assetPath)) return false;
    
    std::string name = fs::path(assetPath).stem().string();
    AssetMetadata* meta = getMetadata(name);
    
    if (!meta) return false;
    
    // Check if file has been modified
    long long currentModTime = getFileModifiedTime(assetPath);
    return currentModTime == meta->lastModified;
}

bool AssetPipeline::needsReprocessing(const std::string& assetPath) const {
    return !validateAsset(assetPath);
}

std::string AssetPipeline::calculateChecksum(const std::string& filePath) const {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return "";
    
    // Simple checksum (in production, use MD5 or SHA256)
    size_t checksum = 0;
    char buffer[4096];
    
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        for (std::streamsize i = 0; i < file.gcount(); i++) {
            checksum = checksum * 31 + static_cast<unsigned char>(buffer[i]);
        }
    }
    
    return std::to_string(checksum);
}

long long AssetPipeline::getFileModifiedTime(const std::string& filePath) const {
    try {
        auto ftime = fs::last_write_time(filePath);
        return ftime.time_since_epoch().count();
    }
    catch (const std::exception&) {
        return 0;
    }
}

size_t AssetPipeline::getFileSize(const std::string& filePath) const {
    try {
        return fs::file_size(filePath);
    }
    catch (const std::exception&) {
        return 0;
    }
}

void AssetPipeline::clear() {
    m_metadata.clear();
}

// ============================================================================
// AssetHotReloader Implementation
// ============================================================================

AssetHotReloader::AssetHotReloader()
    : m_enabled(true) {
}

AssetHotReloader& AssetHotReloader::getInstance() {
    static AssetHotReloader instance;
    return instance;
}

void AssetHotReloader::watchDirectory(const std::string& directory) {
    if (std::find(m_watchedDirectories.begin(), m_watchedDirectories.end(), directory) != m_watchedDirectories.end()) {
        return; // Already watching
    }
    
    m_watchedDirectories.push_back(directory);
    
    // Initialize watched files
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            WatchedFile wf;
            wf.path = entry.path().string();
            wf.lastModified = AssetPipeline::getInstance().getFileModifiedTime(wf.path);
            m_watchedFiles[wf.path] = wf;
        }
    }
    
    std::cout << "Watching directory: " << directory << " (" << m_watchedFiles.size() << " files)" << std::endl;
}

void AssetHotReloader::unwatchDirectory(const std::string& directory) {
    m_watchedDirectories.erase(
        std::remove(m_watchedDirectories.begin(), m_watchedDirectories.end(), directory),
        m_watchedDirectories.end()
    );
}

void AssetHotReloader::update() {
    if (!m_enabled) return;
    
    for (auto& pair : m_watchedFiles) {
        WatchedFile& wf = pair.second;
        long long currentModTime = AssetPipeline::getInstance().getFileModifiedTime(wf.path);
        
        if (currentModTime != wf.lastModified) {
            std::cout << "File changed: " << wf.path << std::endl;
            
            // Trigger callback
            std::string extension = fs::path(wf.path).extension().string();
            auto it = m_callbacks.find(extension);
            if (it != m_callbacks.end()) {
                it->second(wf.path);
            }
            
            wf.lastModified = currentModTime;
        }
    }
}

void AssetHotReloader::registerCallback(const std::string& extension, ReloadCallback callback) {
    m_callbacks[extension] = callback;
}

// ============================================================================
// AssetBundler Implementation
// ============================================================================

AssetBundler::AssetBundler()
    : m_compressionEnabled(true)
    , m_encryptionEnabled(false) {
}

AssetBundler& AssetBundler::getInstance() {
    static AssetBundler instance;
    return instance;
}

bool AssetBundler::createBundle(const std::string& bundleName, const std::vector<std::string>& assetPaths) {
    std::ofstream bundle(bundleName, std::ios::binary);
    if (!bundle.is_open()) {
        std::cerr << "Failed to create bundle: " << bundleName << std::endl;
        return false;
    }
    
    // Write header
    int fileCount = static_cast<int>(assetPaths.size());
    bundle.write(reinterpret_cast<const char*>(&fileCount), sizeof(int));
    
    // Write files
    for (const auto& path : assetPaths) {
        if (!fs::exists(path)) {
            std::cerr << "Asset not found: " << path << std::endl;
            continue;
        }
        
        std::ifstream file(path, std::ios::binary);
        if (!file) continue;
        
        // Write filename length and name
        std::string filename = fs::path(path).filename().string();
        int nameLen = static_cast<int>(filename.length());
        bundle.write(reinterpret_cast<const char*>(&nameLen), sizeof(int));
        bundle.write(filename.c_str(), nameLen);
        
        // Write file size
        size_t fileSize = AssetPipeline::getInstance().getFileSize(path);
        bundle.write(reinterpret_cast<const char*>(&fileSize), sizeof(size_t));
        
        // Write file data
        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        bundle.write(buffer.data(), fileSize);
        
        file.close();
    }
    
    bundle.close();
    std::cout << "Created bundle: " << bundleName << " with " << fileCount << " files" << std::endl;
    return true;
}

bool AssetBundler::extractBundle(const std::string& bundlePath, const std::string& outputDir) {
    std::ifstream bundle(bundlePath, std::ios::binary);
    if (!bundle.is_open()) {
        std::cerr << "Failed to open bundle: " << bundlePath << std::endl;
        return false;
    }
    
    fs::create_directories(outputDir);
    
    // Read header
    int fileCount = 0;
    bundle.read(reinterpret_cast<char*>(&fileCount), sizeof(int));
    
    // Extract files
    for (int i = 0; i < fileCount; i++) {
        // Read filename
        int nameLen = 0;
        bundle.read(reinterpret_cast<char*>(&nameLen), sizeof(int));
        std::string filename(nameLen, '\0');
        bundle.read(&filename[0], nameLen);
        
        // Read file size
        size_t fileSize = 0;
        bundle.read(reinterpret_cast<char*>(&fileSize), sizeof(size_t));
        
        // Read file data
        std::vector<char> buffer(fileSize);
        bundle.read(buffer.data(), fileSize);
        
        // Write to output
        std::string outputPath = outputDir + "/" + filename;
        std::ofstream outFile(outputPath, std::ios::binary);
        outFile.write(buffer.data(), fileSize);
        outFile.close();
        
        std::cout << "Extracted: " << filename << std::endl;
    }
    
    bundle.close();
    std::cout << "Extracted " << fileCount << " files to " << outputDir << std::endl;
    return true;
}

bool AssetBundler::addToBundle(const std::string& bundlePath, const std::string& assetPath) {
    // In production, this would append to existing bundle
    // For simplicity, we'll recreate the bundle
    auto contents = listBundleContents(bundlePath);
    contents.push_back(assetPath);
    return createBundle(bundlePath, contents);
}

std::vector<std::string> AssetBundler::listBundleContents(const std::string& bundlePath) {
    std::vector<std::string> contents;
    
    std::ifstream bundle(bundlePath, std::ios::binary);
    if (!bundle.is_open()) return contents;
    
    int fileCount = 0;
    bundle.read(reinterpret_cast<char*>(&fileCount), sizeof(int));
    
    for (int i = 0; i < fileCount; i++) {
        int nameLen = 0;
        bundle.read(reinterpret_cast<char*>(&nameLen), sizeof(int));
        std::string filename(nameLen, '\0');
        bundle.read(&filename[0], nameLen);
        contents.push_back(filename);
        
        // Skip file data
        size_t fileSize = 0;
        bundle.read(reinterpret_cast<char*>(&fileSize), sizeof(size_t));
        bundle.seekg(fileSize, std::ios::cur);
    }
    
    bundle.close();
    return contents;
}
