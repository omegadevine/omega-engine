#include "ParticleSystem.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// ParticleEmitter Implementation
// ============================================================================

ParticleEmitter::ParticleEmitter(const ParticleEmitterConfig& config)
    : m_config(config)
    , m_emissionTimer(0.0f)
    , m_rng(std::random_device{}())
    , m_dist(0.0f, 1.0f) {
    
    m_particles.resize(config.maxParticles);
    for (auto& particle : m_particles) {
        particle.active = false;
    }
}

float ParticleEmitter::randomRange(float min, float max) {
    return min + m_dist(m_rng) * (max - min);
}

void ParticleEmitter::emitParticle() {
    // Find inactive particle
    for (auto& particle : m_particles) {
        if (!particle.active) {
            particle.active = true;
            particle.position = m_config.position;
            
            // Random velocity based on emission angle and spread
            float angleRad = (m_config.emissionAngle + randomRange(-m_config.emissionSpread, m_config.emissionSpread)) * 3.14159f / 180.0f;
            float speed = randomRange(m_config.minSpeed, m_config.maxSpeed);
            particle.velocity.x = std::cos(angleRad) * speed;
            particle.velocity.y = std::sin(angleRad) * speed;
            
            particle.acceleration = Vector2(0, 0);
            particle.maxLifetime = randomRange(m_config.minLifetime, m_config.maxLifetime);
            particle.lifetime = particle.maxLifetime;
            particle.size = randomRange(m_config.minSize, m_config.maxSize);
            particle.rotation = randomRange(0.0f, 360.0f);
            particle.rotationSpeed = randomRange(m_config.minRotationSpeed, m_config.maxRotationSpeed);
            particle.color = m_config.startColor;
            
            break;
        }
    }
}

void ParticleEmitter::emit(int count) {
    for (int i = 0; i < count; i++) {
        emitParticle();
    }
}

void ParticleEmitter::burst() {
    emit(m_config.burstCount);
}

void ParticleEmitter::clear() {
    for (auto& particle : m_particles) {
        particle.active = false;
    }
}

void ParticleEmitter::update(float deltaTime) {
    // Continuous emission
    if (!m_config.burst) {
        m_emissionTimer += deltaTime;
        float emissionInterval = 1.0f / m_config.emissionRate;
        
        while (m_emissionTimer >= emissionInterval) {
            emitParticle();
            m_emissionTimer -= emissionInterval;
        }
    }
    
    // Update particles
    for (auto& particle : m_particles) {
        if (!particle.active) continue;
        
        // Update lifetime
        particle.lifetime -= deltaTime;
        if (particle.lifetime <= 0.0f) {
            particle.active = false;
            continue;
        }
        
        // Update physics
        particle.acceleration = m_config.gravity;
        particle.velocity.x += particle.acceleration.x * deltaTime;
        particle.velocity.y += particle.acceleration.y * deltaTime;
        particle.velocity.x *= m_config.damping;
        particle.velocity.y *= m_config.damping;
        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;
        
        // Update rotation
        particle.rotation += particle.rotationSpeed * deltaTime;
        
        // Update color (lerp from start to end)
        float t = 1.0f - (particle.lifetime / particle.maxLifetime);
        if (m_config.fadeOut) {
            particle.color.r = m_config.startColor.r + (m_config.endColor.r - m_config.startColor.r) * t;
            particle.color.g = m_config.startColor.g + (m_config.endColor.g - m_config.startColor.g) * t;
            particle.color.b = m_config.startColor.b + (m_config.endColor.b - m_config.startColor.b) * t;
            particle.color.a = m_config.startColor.a + (m_config.endColor.a - m_config.startColor.a) * t;
        }
        
        // Update size
        if (m_config.shrink) {
            particle.size = particle.size * (1.0f - deltaTime / particle.maxLifetime);
        }
    }
}

void ParticleEmitter::render(Shader* shader, int screenWidth, int screenHeight) {
    Sprite particleSprite;
    
    for (const auto& particle : m_particles) {
        if (!particle.active) continue;
        
        particleSprite.setPosition(particle.position);
        particleSprite.setSize(Vector2(particle.size, particle.size));
        particleSprite.setRotation(particle.rotation);
        particleSprite.setColor(particle.color);
        particleSprite.draw(shader, screenWidth, screenHeight);
    }
}

bool ParticleEmitter::isActive() const {
    return getActiveParticleCount() > 0;
}

int ParticleEmitter::getActiveParticleCount() const {
    int count = 0;
    for (const auto& particle : m_particles) {
        if (particle.active) count++;
    }
    return count;
}

// ============================================================================
// ParticleSystem Implementation
// ============================================================================

ParticleSystem::ParticleSystem() {
}

ParticleEmitter* ParticleSystem::addEmitter(const ParticleEmitterConfig& config) {
    auto emitter = std::make_unique<ParticleEmitter>(config);
    ParticleEmitter* ptr = emitter.get();
    m_emitters.push_back(std::move(emitter));
    return ptr;
}

void ParticleSystem::removeEmitter(ParticleEmitter* emitter) {
    m_emitters.erase(
        std::remove_if(m_emitters.begin(), m_emitters.end(),
            [emitter](const std::unique_ptr<ParticleEmitter>& e) {
                return e.get() == emitter;
            }),
        m_emitters.end()
    );
}

void ParticleSystem::clear() {
    m_emitters.clear();
}

void ParticleSystem::update(float deltaTime) {
    for (auto& emitter : m_emitters) {
        emitter->update(deltaTime);
    }
}

void ParticleSystem::render(Shader* shader, int screenWidth, int screenHeight) {
    for (auto& emitter : m_emitters) {
        emitter->render(shader, screenWidth, screenHeight);
    }
}

int ParticleSystem::getTotalParticleCount() const {
    int count = 0;
    for (const auto& emitter : m_emitters) {
        count += emitter->getActiveParticleCount();
    }
    return count;
}

// ============================================================================
// Preset Effects
// ============================================================================

ParticleEmitterConfig ParticleSystem::createFireEffect() {
    ParticleEmitterConfig config;
    config.emissionRate = 50.0f;
    config.emissionAngle = 90.0f;
    config.emissionSpread = 20.0f;
    config.minSpeed = 30.0f;
    config.maxSpeed = 60.0f;
    config.minLifetime = 0.5f;
    config.maxLifetime = 1.5f;
    config.minSize = 5.0f;
    config.maxSize = 15.0f;
    config.gravity = Vector2(0, 50.0f); // Float upward
    config.damping = 0.98f;
    config.startColor = Color(1.0f, 0.8f, 0.2f, 1.0f); // Orange
    config.endColor = Color(1.0f, 0.2f, 0.0f, 0.0f);   // Red fade
    config.fadeOut = true;
    config.shrink = true;
    config.maxParticles = 100;
    return config;
}

ParticleEmitterConfig ParticleSystem::createSmokeEffect() {
    ParticleEmitterConfig config;
    config.emissionRate = 20.0f;
    config.emissionAngle = 90.0f;
    config.emissionSpread = 30.0f;
    config.minSpeed = 20.0f;
    config.maxSpeed = 40.0f;
    config.minLifetime = 2.0f;
    config.maxLifetime = 4.0f;
    config.minSize = 10.0f;
    config.maxSize = 20.0f;
    config.gravity = Vector2(0, 20.0f); // Float upward slowly
    config.damping = 0.99f;
    config.startColor = Color(0.5f, 0.5f, 0.5f, 0.8f); // Gray
    config.endColor = Color(0.3f, 0.3f, 0.3f, 0.0f);   // Dark fade
    config.fadeOut = true;
    config.shrink = false;
    config.maxParticles = 150;
    return config;
}

ParticleEmitterConfig ParticleSystem::createExplosionEffect() {
    ParticleEmitterConfig config;
    config.emissionRate = 0.0f; // Not used
    config.burst = true;
    config.burstCount = 100;
    config.emissionAngle = 0.0f;
    config.emissionSpread = 180.0f; // Full circle
    config.minSpeed = 100.0f;
    config.maxSpeed = 200.0f;
    config.minLifetime = 0.5f;
    config.maxLifetime = 1.5f;
    config.minSize = 3.0f;
    config.maxSize = 8.0f;
    config.gravity = Vector2(0, -200.0f); // Gravity down
    config.damping = 0.95f;
    config.startColor = Color(1.0f, 0.9f, 0.3f, 1.0f); // Yellow
    config.endColor = Color(1.0f, 0.3f, 0.1f, 0.0f);   // Red fade
    config.fadeOut = true;
    config.shrink = true;
    config.maxParticles = 100;
    return config;
}

ParticleEmitterConfig ParticleSystem::createSparkEffect() {
    ParticleEmitterConfig config;
    config.emissionRate = 100.0f;
    config.emissionAngle = 90.0f;
    config.emissionSpread = 90.0f;
    config.minSpeed = 80.0f;
    config.maxSpeed = 150.0f;
    config.minLifetime = 0.3f;
    config.maxLifetime = 0.8f;
    config.minSize = 2.0f;
    config.maxSize = 4.0f;
    config.gravity = Vector2(0, -300.0f);
    config.damping = 0.97f;
    config.startColor = Color(1.0f, 1.0f, 0.8f, 1.0f); // Bright white-yellow
    config.endColor = Color(1.0f, 0.5f, 0.0f, 0.0f);   // Orange fade
    config.fadeOut = true;
    config.shrink = true;
    config.maxParticles = 200;
    return config;
}

ParticleEmitterConfig ParticleSystem::createRainEffect() {
    ParticleEmitterConfig config;
    config.emissionRate = 100.0f;
    config.emissionAngle = 270.0f; // Down
    config.emissionSpread = 5.0f;
    config.minSpeed = 200.0f;
    config.maxSpeed = 300.0f;
    config.minLifetime = 1.0f;
    config.maxLifetime = 2.0f;
    config.minSize = 2.0f;
    config.maxSize = 3.0f;
    config.minRotationSpeed = 0.0f;
    config.maxRotationSpeed = 0.0f;
    config.gravity = Vector2(0, -100.0f);
    config.damping = 1.0f; // No damping
    config.startColor = Color(0.5f, 0.5f, 1.0f, 0.6f);
    config.endColor = Color(0.5f, 0.5f, 1.0f, 0.0f);
    config.fadeOut = true;
    config.shrink = false;
    config.maxParticles = 500;
    return config;
}

ParticleEmitterConfig ParticleSystem::createSnowEffect() {
    ParticleEmitterConfig config;
    config.emissionRate = 50.0f;
    config.emissionAngle = 270.0f; // Down
    config.emissionSpread = 10.0f;
    config.minSpeed = 30.0f;
    config.maxSpeed = 50.0f;
    config.minLifetime = 3.0f;
    config.maxLifetime = 5.0f;
    config.minSize = 3.0f;
    config.maxSize = 6.0f;
    config.gravity = Vector2(0, -20.0f); // Slow fall
    config.damping = 0.99f;
    config.startColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    config.endColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
    config.fadeOut = true;
    config.shrink = false;
    config.maxParticles = 300;
    return config;
}
