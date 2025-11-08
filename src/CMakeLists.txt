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
)

set(HEADERS
    Renderer.h
    Shader.h
    Texture.h
    Sprite.h
    ECS.h
    Input.h
    stb_image.h
)

# executable
add_executable(omega-engine ${SOURCES} ${HEADERS})

# Find SDL2 (system packages)
find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    target_include_directories(omega-engine PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(omega-engine PRIVATE ${SDL2_LIBRARIES})
else()
    message(WARNING "SDL2 not found by find_package. You may need to install SDL2 development packages or modify the build to locate SDL2.")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(omega-engine PRIVATE OpenGL::GL)

# Basic compile flags
target_compile_features(omega-engine PRIVATE cxx_std_17)

# Enable warnings
if(MSVC)
    target_compile_options(omega-engine PRIVATE /W4)
else()
    target_compile_options(omega-engine PRIVATE -Wall -Wextra -pedantic)
endif()
