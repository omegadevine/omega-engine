cmake_minimum_required(VERSION 3.15)

# Source files
set(SOURCES
    main.cpp
    Renderer.cpp
    Shader.cpp
    Texture.cpp
    Sprite.cpp
    ECS.cpp
    Input.cpp
    AssetManager.cpp
    Camera.cpp
    Animation.cpp
    AnimatedSprite.cpp
    Collision.cpp
    Scene.cpp
    SceneManager.cpp
    ExampleScenes.cpp
    Audio.cpp
    UI.cpp
    ParticleSystem.cpp
    Tilemap.cpp
    Text.cpp
)

set(HEADERS
    Renderer.h
    Shader.h
    Texture.h
    Sprite.h
    ECS.h
    Input.h
    AssetManager.h
    Camera.h
    Animation.h
    AnimatedSprite.h
    Collision.h
    Scene.h
    SceneManager.h
    ExampleScenes.h
    Audio.h
    UI.h
    ParticleSystem.h
    Tilemap.h
    Text.h
    stb_image.h
)

# executable
add_executable(omega-engine ${SOURCES} ${HEADERS})

# Find SDL2 (system packages)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(omega-engine PRIVATE SDL2::SDL2main SDL2::SDL2)

# Find SDL2_mixer
find_package(SDL2_mixer CONFIG REQUIRED)
target_link_libraries(omega-engine PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)

# Find OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(omega-engine PRIVATE OpenGL::GL)

# Find GLEW
find_package(GLEW REQUIRED)
target_link_libraries(omega-engine PRIVATE GLEW::GLEW)

# Basic compile flags
target_compile_features(omega-engine PRIVATE cxx_std_17)

# Enable warnings
if(MSVC)
    target_compile_options(omega-engine PRIVATE /W4)
else()
    target_compile_options(omega-engine PRIVATE -Wall -Wextra -pedantic)
endif()
