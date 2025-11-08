#ifndef OMEGA_SAVE_SYSTEM_H
#define OMEGA_SAVE_SYSTEM_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

class SaveData {
public:
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setString(const std::string& key, const std::string& value);
    void setBool(const std::string& key, bool value);
    
    int getInt(const std::string& key, int defaultValue = 0) const;
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    
    bool hasKey(const std::string& key) const;
    void removeKey(const std::string& key);
    void clear();
    
    std::string serialize() const;
    bool deserialize(const std::string& data);

private:
    std::map<std::string, std::string> m_data;
};

class SaveSystem {
public:
    static SaveSystem& getInstance();
    
    bool save(const std::string& slotName, const SaveData& data);
    bool load(const std::string& slotName, SaveData& data);
    bool deleteSlot(const std::string& slotName);
    bool slotExists(const std::string& slotName) const;
    
    std::vector<std::string> listSlots() const;
    
    void setSaveDirectory(const std::string& directory);
    std::string getSaveDirectory() const { return m_saveDirectory; }
    
    void enableEncryption(bool enable, const std::string& key = "");
    void enableCompression(bool enable);
    
    bool autoSave(const SaveData& data);
    bool loadAutoSave(SaveData& data);
    bool hasAutoSave() const;
    
    struct SaveMetadata {
        std::string slotName;
        std::string timestamp;
        std::string version;
        long fileSize;
    };
    
    SaveMetadata getMetadata(const std::string& slotName) const;

private:
    SaveSystem();
    ~SaveSystem() = default;
    SaveSystem(const SaveSystem&) = delete;
    SaveSystem& operator=(const SaveSystem&) = delete;
    
    std::string getSlotFilePath(const std::string& slotName) const;
    bool writeToFile(const std::string& filepath, const std::string& data);
    bool readFromFile(const std::string& filepath, std::string& data);
    
    std::string encrypt(const std::string& data) const;
    std::string decrypt(const std::string& data) const;
    std::string compress(const std::string& data) const;
    std::string decompress(const std::string& data) const;
    
    std::string m_saveDirectory;
    bool m_encryptionEnabled;
    std::string m_encryptionKey;
    bool m_compressionEnabled;
};

class ISaveable {
public:
    virtual ~ISaveable() = default;
    virtual void saveToData(SaveData& data) const = 0;
    virtual void loadFromData(const SaveData& data) = 0;
};

class SaveableRegistry {
public:
    static SaveableRegistry& getInstance();
    
    void registerSaveable(const std::string& id, ISaveable* saveable);
    void unregisterSaveable(const std::string& id);
    
    void saveAll(SaveData& data) const;
    void loadAll(const SaveData& data);
    
    void clear();

private:
    SaveableRegistry() = default;
    ~SaveableRegistry() = default;
    SaveableRegistry(const SaveableRegistry&) = delete;
    SaveableRegistry& operator=(const SaveableRegistry&) = delete;
    
    std::map<std::string, ISaveable*> m_saveables;
};

#endif // OMEGA_SAVE_SYSTEM_H
