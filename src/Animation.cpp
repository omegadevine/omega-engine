#include "Animation.h"

Animation::Animation()
    : m_name("unnamed")
    , m_frameDuration(0.1f)
    , m_loop(true)
    , m_onComplete(nullptr) {
}

Animation::Animation(const std::string& name)
    : m_name(name)
    , m_frameDuration(0.1f)
    , m_loop(true)
    , m_onComplete(nullptr) {
}

void Animation::addFrame(const AnimFrame& frame) {
    m_frames.push_back(frame);
}

void Animation::addFrame(int x, int y, int width, int height) {
    m_frames.push_back(AnimFrame(x, y, width, height));
}

AnimFrame Animation::getFrame(size_t index) const {
    if (index < m_frames.size()) {
        return m_frames[index];
    }
    return AnimFrame();
}
