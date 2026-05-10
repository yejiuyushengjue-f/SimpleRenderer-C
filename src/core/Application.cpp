#include "core/Application.h"

#include <algorithm>

namespace sr {

/// <summary>
/// 初始化帧缓冲、窗口和主循环计时器。
/// </summary>
Application::Application(void* nativeInstance, int showCommand)
    : framebuffer_(960, 540)
    , window_(nativeInstance, showCommand, framebuffer_.width(), framebuffer_.height(), "SimpleRenderer")
    , lastFrameTime_(std::chrono::steady_clock::now())
{
}

/// <summary>
/// 主循环：处理窗口消息、计算帧间隔、更新场景并渲染。
/// </summary>
int Application::run()
{
    while (window_.processMessages()) {
        // 获取当前时间并计算上一帧到当前帧的时间差。
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = now - lastFrameTime_;
        lastFrameTime_ = now;

        // 限制 delta time，避免调试暂停或卡顿后产生过大的更新步长。
        const float deltaSeconds = std::min(elapsed.count(), 0.1f);

        update(deltaSeconds);
        render();
    }

    return 0;
}

/// <summary>
/// 将帧时间传给测试场景，让动画按时间推进。
/// </summary>
void Application::update(float deltaSeconds)
{
    scene_.update(deltaSeconds);
}

/// <summary>
/// 渲染到离屏帧缓冲，然后呈现到窗口。
/// </summary>
void Application::render()
{
    renderer_.render(scene_, framebuffer_);
    window_.present(framebuffer_);
}

} // namespace sr
