#include <SDL.h>
#include <iostream>
#include <cstring>

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

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // TODO: initialize OpenGL and clear screen here
        SDL_Delay(16); // naive frame limiter (~60fps)
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
