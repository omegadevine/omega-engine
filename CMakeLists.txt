cmake_minimum_required(VERSION 3.15)
project(omega-engine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: allow user to set BUILD_TESTING
option(BUILD_TESTING "Build tests" ON)

add_subdirectory(src)
