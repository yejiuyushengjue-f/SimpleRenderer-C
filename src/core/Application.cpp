#include "core/Application.h"

#include <algorithm>
#include <string>

namespace sr {

Application::Application(void* nativeInstance, int showCommand)
    : framebuffer_(960, 540)
    , window_(nativeInstance, showCommand, framebuffer_.width(), framebuffer_.height(), "SimpleRenderer")
    , lastFrameTime_(std::chrono::steady_clock::now())
{
    window_.setTitle("SimpleRenderer - Render Mode: Final");
    resizeFramebufferToWindow();
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
    const InputState input = window_.inputState();
    if (input.toggleFullscreen) {
        window_.toggleFullscreen();
    }
    resizeFramebufferToWindow();

    camera_.update(input, deltaSeconds);
    if (input.renderModeSelection > 0) {
        renderer_.setRenderMode(static_cast<RenderMode>(input.renderModeSelection - 1));
    }
    if (renderer_.renderMode() != lastWindowTitleMode_) {
        std::string title = "SimpleRenderer - Render Mode: ";
        title += renderer_.renderModeName();
        window_.setTitle(title.c_str());
        lastWindowTitleMode_ = renderer_.renderMode();
    }
    scene_.update(deltaSeconds);
}

void Application::resizeFramebufferToWindow()
{
    int width = 0;
    int height = 0;
    if (window_.clientSize(width, height)) {
        framebuffer_.resize(width, height);
    }
}

void Application::render()
{
    renderer_.render(scene_, camera_, framebuffer_);
    window_.present(framebuffer_);
}

} // namespace sr
