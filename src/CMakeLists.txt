cmake_minimum_required(VERSION 3.15)

# executable
add_executable(omega-engine main.cpp)

# Find SDL2 (system packages)
find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    target_include_directories(omega-engine PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(omega-engine PRIVATE ${SDL2_LIBRARIES})
else()
    message(WARNING "SDL2 not found by find_package. You may need to install SDL2 development packages or modify the build to locate SDL2.")
endif()

# Basic compile flags
target_compile_features(omega-engine PRIVATE cxx_std_17)
