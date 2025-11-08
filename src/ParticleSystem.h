#ifndef OMEGA_PARTICLE_SYSTEM_H
#define OMEGA_PARTICLE_SYSTEM_H

#include "Sprite.h"
#include "Shader.h"
#include <vector>
#include <random>

// Single particle
struct Particle {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Color color;
    float lifetime;
    float maxLifetime;
    float size;
    float rotation;
    float rotationSpeed;
    bool active;
};

// Particle emitter configuration
struct ParticleEmitterConfig {
    // Emission
    Vector2 position;
    float emissionRate;        // particles per second
    float emissionAngle;       // center angle in degrees
    float emissionSpread;      // spread angle in degrees
    
    // Particle properties
    float minSpeed;
    float maxSpeed;
    float minLifetime;
    float maxLifetime;
    float minSize;
    float maxSize;
    float minRotationSpeed;
    float maxRotationSpeed;
    
    // Physics
    Vector2 gravity;
    float damping;
    
    // Colors
    Color startColor;
    Color endColor;
    
    // Behavior
    bool fadeOut;
    bool shrink;
    int maxParticles;
    bool burst;               // emit all at once
    int burstCount;
    
    ParticleEmitterConfig()
        : position(0, 0)
        , emissionRate(10.0f)
        , emissionAngle(90.0f)
        , emissionSpread(30.0f)
        , minSpeed(50.0f)
        , maxSpeed(100.0f)
        , minLifetime(1.0f)
        , maxLifetime(2.0f)
        , minSize(5.0f)
        , maxSize(10.0f)
        , minRotationSpeed(-180.0f)
        , maxRotationSpeed(180.0f)
        , gravity(0, -100.0f)
        , damping(0.95f)
        , startColor(1.0f, 1.0f, 1.0f, 1.0f)
        , endColor(1.0f, 1.0f, 1.0f, 0.0f)
        , fadeOut(true)
        , shrink(false)
        , maxParticles(100)
        , burst(false)
        , burstCount(50) {
    }
};

// Particle emitter
class ParticleEmitter {
public:
    ParticleEmitter(const ParticleEmitterConfig& config);
    ~ParticleEmitter() = default;
    
    void update(float deltaTime);
    void render(Shader* shader, int screenWidth, int screenHeight);
    
    void setPosition(const Vector2& pos) { m_config.position = pos; }
    void emit(int count = 1);
    void burst();
    void clear();
    
    bool isActive() const;
    int getActiveParticleCount() const;
    
    ParticleEmitterConfig& getConfig() { return m_config; }

private:
    void emitParticle();
    float randomRange(float min, float max);
    
    ParticleEmitterConfig m_config;
    std::vector<Particle> m_particles;
    float m_emissionTimer;
    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_dist;
};

// Particle system manager
class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem() = default;
    
    ParticleEmitter* addEmitter(const ParticleEmitterConfig& config);
    void removeEmitter(ParticleEmitter* emitter);
    void clear();
    
    void update(float deltaTime);
    void render(Shader* shader, int screenWidth, int screenHeight);
    
    size_t getEmitterCount() const { return m_emitters.size(); }
    int getTotalParticleCount() const;
    
    // Preset effects
    static ParticleEmitterConfig createFireEffect();
    static ParticleEmitterConfig createSmokeEffect();
    static ParticleEmitterConfig createExplosionEffect();
    static ParticleEmitterConfig createSparkEffect();
    static ParticleEmitterConfig createRainEffect();
    static ParticleEmitterConfig createSnowEffect();

private:
    std::vector<std::unique_ptr<ParticleEmitter>> m_emitters;
};

#endif // OMEGA_PARTICLE_SYSTEM_H
