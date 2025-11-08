#include "Audio.h"
#include <iostream>

AudioManager::~AudioManager() {
    shutdown();
}

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

bool AudioManager::initialize(int frequency, int channels, int chunkSize) {
    if (m_initialized) {
        std::cerr << "AudioManager: Already initialized" << std::endl;
        return true;
    }
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, chunkSize) < 0) {
        std::cerr << "AudioManager: Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Allocate mixing channels for sound effects
    Mix_AllocateChannels(16); // 16 simultaneous sounds
    
    m_initialized = true;
    m_musicVolume = MIX_MAX_VOLUME;
    m_soundVolume = MIX_MAX_VOLUME;
    
    std::cout << "AudioManager: Initialized successfully" << std::endl;
    std::cout << "  Frequency: " << frequency << " Hz" << std::endl;
    std::cout << "  Channels: " << channels << std::endl;
    std::cout << "  Chunk size: " << chunkSize << std::endl;
    
    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) return;
    
    std::cout << "AudioManager: Shutting down..." << std::endl;
    
    unloadAll();
    Mix_CloseAudio();
    
    m_initialized = false;
}

bool AudioManager::loadMusic(const std::string& name, const std::string& filepath) {
    if (!m_initialized) {
        std::cerr << "AudioManager: Not initialized" << std::endl;
        return false;
    }
    
    // Check if already loaded
    if (m_music.find(name) != m_music.end()) {
        std::cout << "AudioManager: Music '" << name << "' already loaded" << std::endl;
        return true;
    }
    
    Mix_Music* music = Mix_LoadMUS(filepath.c_str());
    if (!music) {
        std::cerr << "AudioManager: Failed to load music '" << name << "': " << Mix_GetError() << std::endl;
        return false;
    }
    
    m_music[name] = music;
    std::cout << "AudioManager: Loaded music '" << name << "'" << std::endl;
    return true;
}

void AudioManager::playMusic(const std::string& name, int loops) {
    if (!m_initialized) return;
    
    auto it = m_music.find(name);
    if (it == m_music.end()) {
        std::cerr << "AudioManager: Music '" << name << "' not found" << std::endl;
        return;
    }
    
    if (Mix_PlayMusic(it->second, loops) == -1) {
        std::cerr << "AudioManager: Failed to play music '" << name << "': " << Mix_GetError() << std::endl;
        return;
    }
    
    std::cout << "AudioManager: Playing music '" << name << "'" << std::endl;
}

void AudioManager::pauseMusic() {
    if (m_initialized) {
        Mix_PauseMusic();
    }
}

void AudioManager::resumeMusic() {
    if (m_initialized) {
        Mix_ResumeMusic();
    }
}

void AudioManager::stopMusic() {
    if (m_initialized) {
        Mix_HaltMusic();
    }
}

void AudioManager::fadeInMusic(const std::string& name, int ms, int loops) {
    if (!m_initialized) return;
    
    auto it = m_music.find(name);
    if (it == m_music.end()) {
        std::cerr << "AudioManager: Music '" << name << "' not found" << std::endl;
        return;
    }
    
    if (Mix_FadeInMusic(it->second, loops, ms) == -1) {
        std::cerr << "AudioManager: Failed to fade in music '" << name << "': " << Mix_GetError() << std::endl;
        return;
    }
    
    std::cout << "AudioManager: Fading in music '" << name << "' over " << ms << "ms" << std::endl;
}

void AudioManager::fadeOutMusic(int ms) {
    if (m_initialized) {
        Mix_FadeOutMusic(ms);
    }
}

bool AudioManager::loadSound(const std::string& name, const std::string& filepath) {
    if (!m_initialized) {
        std::cerr << "AudioManager: Not initialized" << std::endl;
        return false;
    }
    
    // Check if already loaded
    if (m_sounds.find(name) != m_sounds.end()) {
        std::cout << "AudioManager: Sound '" << name << "' already loaded" << std::endl;
        return true;
    }
    
    Mix_Chunk* sound = Mix_LoadWAV(filepath.c_str());
    if (!sound) {
        std::cerr << "AudioManager: Failed to load sound '" << name << "': " << Mix_GetError() << std::endl;
        return false;
    }
    
    m_sounds[name] = sound;
    std::cout << "AudioManager: Loaded sound '" << name << "'" << std::endl;
    return true;
}

void AudioManager::playSound(const std::string& name, int loops) {
    if (!m_initialized) return;
    
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) {
        std::cerr << "AudioManager: Sound '" << name << "' not found" << std::endl;
        return;
    }
    
    int channel = Mix_PlayChannel(-1, it->second, loops);
    if (channel == -1) {
        std::cerr << "AudioManager: Failed to play sound '" << name << "': " << Mix_GetError() << std::endl;
    }
}

void AudioManager::stopSound(int channel) {
    if (m_initialized) {
        Mix_HaltChannel(channel);
    }
}

void AudioManager::setMusicVolume(int volume) {
    m_musicVolume = volume < 0 ? 0 : (volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume);
    if (m_initialized) {
        Mix_VolumeMusic(m_musicVolume);
    }
}

void AudioManager::setSoundVolume(int volume) {
    m_soundVolume = volume < 0 ? 0 : (volume > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : volume);
    if (m_initialized) {
        Mix_Volume(-1, m_soundVolume); // -1 = all channels
    }
}

int AudioManager::getMusicVolume() const {
    return m_musicVolume;
}

int AudioManager::getSoundVolume() const {
    return m_soundVolume;
}

bool AudioManager::isMusicPlaying() const {
    return m_initialized && Mix_PlayingMusic();
}

bool AudioManager::isMusicPaused() const {
    return m_initialized && Mix_PausedMusic();
}

void AudioManager::unloadMusic(const std::string& name) {
    auto it = m_music.find(name);
    if (it != m_music.end()) {
        Mix_FreeMusic(it->second);
        m_music.erase(it);
        std::cout << "AudioManager: Unloaded music '" << name << "'" << std::endl;
    }
}

void AudioManager::unloadSound(const std::string& name) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        Mix_FreeChunk(it->second);
        m_sounds.erase(it);
        std::cout << "AudioManager: Unloaded sound '" << name << "'" << std::endl;
    }
}

void AudioManager::unloadAll() {
    std::cout << "AudioManager: Unloading all audio..." << std::endl;
    
    // Stop all audio
    stopMusic();
    stopSound(-1);
    
    // Free all music
    for (auto& pair : m_music) {
        Mix_FreeMusic(pair.second);
    }
    m_music.clear();
    
    // Free all sounds
    for (auto& pair : m_sounds) {
        Mix_FreeChunk(pair.second);
    }
    m_sounds.clear();
    
    std::cout << "AudioManager: All audio unloaded" << std::endl;
}
