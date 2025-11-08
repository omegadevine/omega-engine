# Advanced Rendering Guide

## Post-Processing Effects

The Omega Engine includes a flexible post-processing system with multiple built-in effects.

### Available Effects

#### Bloom
Adds a glow effect to bright areas of the scene.

```cpp
auto bloom = std::make_shared<BloomEffect>();
bloom->setThreshold(0.8f);  // Brightness threshold
bloom->setIntensity(1.5f);   // Bloom intensity
postProcessStack.addEffect(bloom);
```

#### Color Grading
Adjusts contrast, brightness, and saturation.

```cpp
auto colorGrading = std::make_shared<ColorGradingEffect>();
colorGrading->setContrast(1.2f);
colorGrading->setBrightness(0.1f);
colorGrading->setSaturation(1.1f);
postProcessStack.addEffect(colorGrading);
```

#### Vignette
Darkens the edges of the screen.

```cpp
auto vignette = std::make_shared<VignetteEffect>();
vignette->setIntensity(0.5f);
vignette->setRadius(0.8f);
postProcessStack.addEffect(vignette);
```

#### Chromatic Aberration
Simulates lens distortion with color splitting.

```cpp
auto chromatic = std::make_shared<ChromaticAberrationEffect>();
chromatic->setIntensity(0.01f);
postProcessStack.addEffect(chromatic);
```

### Using the Post-Processing Stack

```cpp
// Initialize
PostProcessingStack postProcessStack(1920, 1080);

// Add effects in order
postProcessStack.addEffect(bloom);
postProcessStack.addEffect(colorGrading);
postProcessStack.addEffect(vignette);

// Render scene to texture
GLuint sceneTexture = renderSceneToTexture();

// Apply all effects
postProcessStack.process(sceneTexture);

// Get final result
GLuint finalTexture = postProcessStack.getFinalTexture();
```

### Custom Effects

Create custom post-processing effects by inheriting from `PostProcessEffect`:

```cpp
class CustomEffect : public PostProcessEffect {
public:
    void apply(GLuint sourceTexture, GLuint targetFramebuffer) override {
        // Your shader logic here
    }
    
    void setParameter(const std::string& name, float value) override {
        // Handle parameter changes
    }
};
```

### Performance Tips

- Effects are applied in order, expensive effects should be last
- Disable effects when not needed: `effect->setEnabled(false)`
- Use lower resolution for expensive effects like bloom
- Consider quality presets for different hardware

### Shader Examples

Bloom bright pass shader:
```glsl
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D scene;
uniform float threshold;

void main() {
    vec3 color = texture(scene, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    FragColor = brightness > threshold ? vec4(color, 1.0) : vec4(0.0);
}
```

## Render Targets

Create and manage custom render targets for advanced rendering techniques:

```cpp
// Create framebuffer
GLuint fbo, texture;
glGenFramebuffers(1, &fbo);
glGenTextures(1, &texture);

// Setup texture
glBindTexture(GL_TEXTURE_2D, texture);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// Attach to framebuffer
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
```

## Multi-Pass Rendering

Implement complex rendering pipelines:

1. **Geometry Pass** - Render scene geometry
2. **Lighting Pass** - Calculate lighting
3. **Post-Processing Pass** - Apply effects
4. **UI Pass** - Render UI on top

```cpp
// Geometry pass
glBindFramebuffer(GL_FRAMEBUFFER, geometryFBO);
renderGeometry();

// Lighting pass
glBindFramebuffer(GL_FRAMEBUFFER, lightingFBO);
applyLighting(geometryTextures);

// Post-processing
postProcessStack.process(lightingTexture);

// Final composite with UI
glBindFramebuffer(GL_FRAMEBUFFER, 0);
renderFinal(postProcessStack.getFinalTexture());
renderUI();
```
