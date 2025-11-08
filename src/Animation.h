#ifndef OMEGA_ANIMATION_H
#define OMEGA_ANIMATION_H

#include <vector>
#include <string>
#include <functional>

// Rectangle for frame coordinates in sprite sheet
struct AnimFrame {
    int x, y, width, height;
    
    AnimFrame(int _x = 0, int _y = 0, int _w = 0, int _h = 0)
        : x(_x), y(_y), width(_w), height(_h) {}
};

// Animation definition
class Animation {
public:
    Animation();
    Animation(const std::string& name);
    
    void addFrame(const AnimFrame& frame);
    void addFrame(int x, int y, int width, int height);
    
    void setFrameDuration(float duration) { m_frameDuration = duration; }
    float getFrameDuration() const { return m_frameDuration; }
    
    void setLoop(bool loop) { m_loop = loop; }
    bool isLooping() const { return m_loop; }
    
    void setName(const std::string& name) { m_name = name; }
    std::string getName() const { return m_name; }
    
    size_t getFrameCount() const { return m_frames.size(); }
    AnimFrame getFrame(size_t index) const;
    
    // Animation events (called when animation completes)
    void setOnComplete(std::function<void()> callback) { m_onComplete = callback; }
    void triggerComplete() { if (m_onComplete) m_onComplete(); }

private:
    std::string m_name;
    std::vector<AnimFrame> m_frames;
    float m_frameDuration;
    bool m_loop;
    std::function<void()> m_onComplete;
};

#endif // OMEGA_ANIMATION_H
