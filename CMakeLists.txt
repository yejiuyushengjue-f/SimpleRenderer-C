cmake_minimum_required(VERSION 3.20)

project(SimpleRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SimpleRenderer WIN32
    src/main.cpp
    src/core/Application.cpp
    src/core/Camera.cpp
    src/core/Framebuffer.cpp
    src/platform/Win32Window.cpp
    src/renderer/Renderer.cpp
    src/renderer/Texture.cpp
    src/scenes/TestScene.cpp
)

target_include_directories(SimpleRenderer PRIVATE src)
target_compile_definitions(SimpleRenderer PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(SimpleRenderer PRIVATE ole32 windowscodecs)

if(MSVC)
    target_compile_options(SimpleRenderer PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(SimpleRenderer PRIVATE -Wall -Wextra -Wpedantic)
endif()
