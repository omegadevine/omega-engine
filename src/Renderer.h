#ifndef OMEGA_RENDERER_H
#define OMEGA_RENDERER_H

#include <SDL.h>
#include <string>

class Renderer {
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    bool initialize();
    void clear(float r, float g, float b, float a);
    void present();
    
    bool isInitialized() const { return m_initialized; }

private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    bool m_initialized;
};

#endif // OMEGA_RENDERER_H
