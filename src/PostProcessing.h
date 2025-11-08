#ifndef OMEGA_POST_PROCESSING_H
#define OMEGA_POST_PROCESSING_H

#include "Shader.h"
#include <GL/glew.h>
#include <string>
#include <memory>

class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;
    virtual void apply(GLuint sourceTexture, GLuint targetFramebuffer) = 0;
    virtual void setParameter(const std::string& name, float value) = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

class BloomEffect : public PostProcessEffect {
public:
    BloomEffect();
    ~BloomEffect() override;
    
    void apply(GLuint sourceTexture, GLuint targetFramebuffer) override;
    void setParameter(const std::string& name, float value) override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
    
    void setThreshold(float threshold) { m_threshold = threshold; }
    void setIntensity(float intensity) { m_intensity = intensity; }

private:
    bool m_enabled;
    float m_threshold;
    float m_intensity;
    std::unique_ptr<Shader> m_brightPassShader;
    std::unique_ptr<Shader> m_blurShader;
    std::unique_ptr<Shader> m_combineShader;
    GLuint m_brightPassFBO;
    GLuint m_blurFBOs[2];
    GLuint m_brightPassTexture;
    GLuint m_blurTextures[2];
};

class ColorGradingEffect : public PostProcessEffect {
public:
    ColorGradingEffect();
    ~ColorGradingEffect() override;
    
    void apply(GLuint sourceTexture, GLuint targetFramebuffer) override;
    void setParameter(const std::string& name, float value) override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
    
    void setContrast(float contrast) { m_contrast = contrast; }
    void setBrightness(float brightness) { m_brightness = brightness; }
    void setSaturation(float saturation) { m_saturation = saturation; }

private:
    bool m_enabled;
    float m_contrast;
    float m_brightness;
    float m_saturation;
    std::unique_ptr<Shader> m_shader;
};

class VignetteEffect : public PostProcessEffect {
public:
    VignetteEffect();
    ~VignetteEffect() override;
    
    void apply(GLuint sourceTexture, GLuint targetFramebuffer) override;
    void setParameter(const std::string& name, float value) override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
    
    void setIntensity(float intensity) { m_intensity = intensity; }
    void setRadius(float radius) { m_radius = radius; }

private:
    bool m_enabled;
    float m_intensity;
    float m_radius;
    std::unique_ptr<Shader> m_shader;
};

class ChromaticAberrationEffect : public PostProcessEffect {
public:
    ChromaticAberrationEffect();
    ~ChromaticAberrationEffect() override;
    
    void apply(GLuint sourceTexture, GLuint targetFramebuffer) override;
    void setParameter(const std::string& name, float value) override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }
    
    void setIntensity(float intensity) { m_intensity = intensity; }

private:
    bool m_enabled;
    float m_intensity;
    std::unique_ptr<Shader> m_shader;
};

class PostProcessingStack {
public:
    PostProcessingStack(int width, int height);
    ~PostProcessingStack();
    
    void addEffect(std::shared_ptr<PostProcessEffect> effect);
    void removeEffect(PostProcessEffect* effect);
    void clearEffects();
    
    void process(GLuint sourceTexture);
    GLuint getFinalTexture() const { return m_finalTexture; }
    
    void resize(int width, int height);
    
private:
    void initializeFramebuffers();
    void cleanupFramebuffers();
    
    int m_width;
    int m_height;
    std::vector<std::shared_ptr<PostProcessEffect>> m_effects;
    GLuint m_pingpongFBOs[2];
    GLuint m_pingpongTextures[2];
    GLuint m_finalTexture;
};

#endif // OMEGA_POST_PROCESSING_H
