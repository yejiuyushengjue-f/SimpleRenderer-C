#pragma once

#include "core/Framebuffer.h"
#include "platform/Win32Window.h"
#include "renderer/Renderer.h"
#include "scenes/TestScene.h"

#include <chrono>

namespace sr {

/// <summary>
/// 应用程序主类，负责管理窗口、帧缓冲、渲染器和测试场景的生命周期。
/// </summary>
class Application {
public:
    /// <summary>
    /// 构造应用程序并初始化主要模块。
    /// </summary>
    /// <param name="nativeInstance">Windows 应用程序实例句柄。</param>
    /// <param name="showCommand">窗口显示方式。</param>
    Application(void* nativeInstance, int showCommand);

    /// <summary>
    /// 启动主循环，持续处理消息、更新场景并渲染帧缓冲。
    /// </summary>
    /// <returns>应用程序退出码。</returns>
    int run();

private:
    /// <summary>
    /// 更新场景逻辑。
    /// </summary>
    /// <param name="deltaSeconds">上一帧到当前帧的时间间隔，单位为秒。</param>
    void update(float deltaSeconds);

    /// <summary>
    /// 渲染场景并将帧缓冲呈现到窗口。
    /// </summary>
    void render();

    /// <summary>离屏帧缓冲，保存软件渲染结果。</summary>
    Framebuffer framebuffer_;

    /// <summary>Windows 平台窗口封装。</summary>
    Win32Window window_;

    /// <summary>软件渲染管线。</summary>
    Renderer renderer_;

    /// <summary>简单测试场景。</summary>
    TestScene scene_;

    /// <summary>上一帧时间戳，用于计算 delta time。</summary>
    std::chrono::steady_clock::time_point lastFrameTime_;
};

} // namespace sr
