#ifndef OMEGA_AUDIO_H
#define OMEGA_AUDIO_H

#include <SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <memory>

// Audio types
enum class AudioType {
    Music,
    SoundEffect
};

// Audio system manager (Singleton)
class AudioManager {
public:
    static AudioManager& getInstance();
    
    // Initialize/shutdown
    bool initialize(int frequency = 44100, int channels = 2, int chunkSize = 2048);
    void shutdown();
    
    // Music management
    bool loadMusic(const std::string& name, const std::string& filepath);
    void playMusic(const std::string& name, int loops = -1); // -1 = infinite loop
    void pauseMusic();
    void resumeMusic();
    void stopMusic();
    void fadeInMusic(const std::string& name, int ms, int loops = -1);
    void fadeOutMusic(int ms);
    
    // Sound effect management
    bool loadSound(const std::string& name, const std::string& filepath);
    void playSound(const std::string& name, int loops = 0); // 0 = play once
    void stopSound(int channel = -1); // -1 = all channels
    
    // Volume control (0-128)
    void setMusicVolume(int volume);
    void setSoundVolume(int volume);
    int getMusicVolume() const;
    int getSoundVolume() const;
    
    // State queries
    bool isMusicPlaying() const;
    bool isMusicPaused() const;
    bool isInitialized() const { return m_initialized; }
    
    // Cleanup
    void unloadMusic(const std::string& name);
    void unloadSound(const std::string& name);
    void unloadAll();
    
    // Statistics
    size_t getMusicCount() const { return m_music.size(); }
    size_t getSoundCount() const { return m_sounds.size(); }

private:
    AudioManager() = default;
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    
    bool m_initialized;
    std::unordered_map<std::string, Mix_Music*> m_music;
    std::unordered_map<std::string, Mix_Chunk*> m_sounds;
    int m_musicVolume;
    int m_soundVolume;
};

#endif // OMEGA_AUDIO_H
