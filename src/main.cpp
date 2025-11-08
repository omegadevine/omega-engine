#include <SDL.h>
#include <iostream>
#include <cstring>
#include "Renderer.h"
#include "Shader.h"

int main(int argc, char** argv) {
    // Input validation for command line arguments
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (argv[i] && std::strlen(argv[i]) > 1024) {
                std::cerr << "Error: Command line argument too long" << std::endl;
                return 1;
            }
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "omega-engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Initialize renderer
    Renderer renderer(window);
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a basic shader for testing
    Shader testShader;
    const std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        void main() {
            gl_Position = vec4(position, 1.0);
        }
    )";
    
    const std::string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.2, 0.6, 0.8, 1.0);
        }
    )";

    if (testShader.loadFromSource(vertexShaderSource, fragmentShaderSource)) {
        std::cout << "Test shader loaded successfully" << std::endl;
    } else {
        std::cerr << "Warning: Test shader failed to load" << std::endl;
    }

    bool running = true;
    SDL_Event event;
    float colorPhase = 0.0f;

    std::cout << "Engine running. Press ESC or close window to quit." << std::endl;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Animate background color
        colorPhase += 0.01f;
        if (colorPhase > 6.28f) colorPhase = 0.0f;

        float r = (std::sin(colorPhase) + 1.0f) * 0.5f;
        float g = (std::sin(colorPhase + 2.09f) + 1.0f) * 0.5f;
        float b = (std::sin(colorPhase + 4.18f) + 1.0f) * 0.5f;

        renderer.clear(r * 0.3f, g * 0.3f, b * 0.3f, 1.0f);
        renderer.present();

        SDL_Delay(16); // ~60fps
    }

    std::cout << "Shutting down..." << std::endl;

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
