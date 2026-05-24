#pragma once

#include "core/Camera.h"
#include "core/Framebuffer.h"
#include "platform/Win32Window.h"
#include "renderer/Renderer.h"
#include "scenes/TestScene.h"

#include <chrono>

namespace sr {

class Application {
public:
    Application(void* nativeInstance, int showCommand);

    int run();

private:
    void update(float deltaSeconds);
    void render();

    Framebuffer framebuffer_;
    Win32Window window_;
    Camera camera_;
    Renderer renderer_;
    TestScene scene_;
    std::chrono::steady_clock::time_point lastFrameTime_;
};

} // namespace sr
