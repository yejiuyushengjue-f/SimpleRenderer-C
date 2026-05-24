#include "core/Application.h"

#include <algorithm>

namespace sr {

Application::Application(void* nativeInstance, int showCommand)
    : framebuffer_(960, 540)
    , window_(nativeInstance, showCommand, framebuffer_.width(), framebuffer_.height(), "SimpleRenderer")
    , lastFrameTime_(std::chrono::steady_clock::now())
{
}

int Application::run()
{
    while (window_.processMessages()) {
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = now - lastFrameTime_;
        lastFrameTime_ = now;

        const float deltaSeconds = std::min(elapsed.count(), 0.1f);
        update(deltaSeconds);
        render();
    }

    return 0;
}

void Application::update(float deltaSeconds)
{
    camera_.update(window_.inputState(), deltaSeconds);
    scene_.update(deltaSeconds);
}

void Application::render()
{
    renderer_.render(scene_, camera_, framebuffer_);
    window_.present(framebuffer_);
}

} // namespace sr
