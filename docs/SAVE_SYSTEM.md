# Save System Guide

## Overview

The Omega Engine save system provides persistent data storage with encryption, compression, and automatic management.

## Quick Start

### Basic Save/Load

```cpp
#include "SaveSystem.h"

// Create save data
SaveData data;
data.setInt("playerLevel", 10);
data.setFloat("playerHealth", 75.5f);
data.setString("playerName", "Hero");
data.setBool("hasCompletedTutorial", true);

// Save to slot
SaveSystem::getInstance().save("slot1", data);

// Load from slot
SaveData loadedData;
if (SaveSystem::getInstance().load("slot1", loadedData)) {
    int level = loadedData.getInt("playerLevel");
    float health = loadedData.getFloat("playerHealth");
    std::string name = loadedData.getString("playerName");
    bool tutorial = loadedData.getBool("hasCompletedTutorial");
}
```

### Auto-Save

```cpp
// Enable auto-save
SaveData autoData;
autoData.setInt("checkpoint", 5);
SaveSystem::getInstance().autoSave(autoData);

// Load auto-save
SaveData loadedAuto;
if (SaveSystem::getInstance().hasAutoSave()) {
    SaveSystem::getInstance().loadAutoSave(loadedAuto);
}
```

## Advanced Features

### Encryption

Protect save files from tampering:

```cpp
SaveSystem::getInstance().enableEncryption(true, "MySecretKey123");
SaveSystem::getInstance().save("slot1", data);
```

### Compression

Reduce save file sizes:

```cpp
SaveSystem::getInstance().enableCompression(true);
SaveSystem::getInstance().save("slot1", data);
```

### Custom Save Directory

```cpp
SaveSystem::getInstance().setSaveDirectory("C:/MyGame/Saves");
```

### Save Metadata

```cpp
auto metadata = SaveSystem::getInstance().getMetadata("slot1");
std::cout << "Slot: " << metadata.slotName << std::endl;
std::cout << "Saved: " << metadata.timestamp << std::endl;
std::cout << "Version: " << metadata.version << std::endl;
std::cout << "Size: " << metadata.fileSize << " bytes" << std::endl;
```

### List All Saves

```cpp
auto slots = SaveSystem::getInstance().listSlots();
for (const auto& slot : slots) {
    std::cout << "Save slot: " << slot << std::endl;
}
```

## Saveable Objects

Implement the `ISaveable` interface for automatic saving:

```cpp
class Player : public ISaveable {
public:
    void saveToData(SaveData& data) const override {
        data.setInt("health", m_health);
        data.setFloat("x", m_position.x);
        data.setFloat("y", m_position.y);
        data.setString("weapon", m_currentWeapon);
    }
    
    void loadFromData(const SaveData& data) override {
        m_health = data.getInt("health");
        m_position.x = data.getFloat("x");
        m_position.y = data.getFloat("y");
        m_currentWeapon = data.getString("weapon");
    }

private:
    int m_health;
    Vector2 m_position;
    std::string m_currentWeapon;
};
```

Register with the SaveableRegistry:

```cpp
Player player;
SaveableRegistry::getInstance().registerSaveable("player", &player);

// Save all registered objects
SaveData data;
SaveableRegistry::getInstance().saveAll(data);
SaveSystem::getInstance().save("slot1", data);

// Load all registered objects
SaveData loadedData;
SaveSystem::getInstance().load("slot1", loadedData);
SaveableRegistry::getInstance().loadAll(loadedData);
```

## Save File Format

Save files use a simple key-value format internally:

```
playerLevel:10
playerHealth:75.5
playerName:Hero
hasCompletedTutorial:true
```

With encryption and compression enabled, files are binary encoded.

## Best Practices

1. **Version your saves**: Include version info for compatibility
```cpp
data.setString("saveVersion", "1.0.0");
```

2. **Validate loaded data**: Check for expected keys
```cpp
if (data.hasKey("playerLevel")) {
    int level = data.getInt("playerLevel");
}
```

3. **Handle missing saves gracefully**:
```cpp
if (!SaveSystem::getInstance().slotExists("slot1")) {
    // Start new game
}
```

4. **Regular auto-saves**: Save progress at checkpoints
```cpp
void onCheckpoint() {
    SaveData checkpoint;
    SaveableRegistry::getInstance().saveAll(checkpoint);
    SaveSystem::getInstance().autoSave(checkpoint);
}
```

5. **Delete old saves**: Clean up when needed
```cpp
SaveSystem::getInstance().deleteSlot("slot1");
```

## Error Handling

```cpp
try {
    if (!SaveSystem::getInstance().save("slot1", data)) {
        std::cerr << "Failed to save game" << std::endl;
    }
} catch (const std::exception& e) {
    std::cerr << "Save error: " << e.what() << std::endl;
}
```

## Platform Support

The save system automatically determines the appropriate save location:

- **Windows**: `%APPDATA%/OmegaEngine/Saves`
- **Linux**: `~/.local/share/OmegaEngine/Saves`
- **macOS**: `~/Library/Application Support/OmegaEngine/Saves`

Override with `setSaveDirectory()` if needed.
