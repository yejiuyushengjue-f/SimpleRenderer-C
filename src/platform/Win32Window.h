#pragma once

#include "core/Framebuffer.h"
#include "platform/InputState.h"

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace sr {

class Win32Window {
public:
    Win32Window(void* nativeInstance, int showCommand, int width, int height, const char* title);
    ~Win32Window();

    bool processMessages();
    InputState inputState();
    void setTitle(const char* title);
    bool clientSize(int& width, int& height) const;
    void toggleFullscreen();
    void present(const Framebuffer& framebuffer);

private:
#ifdef _WIN32
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    HWND hwnd_ = nullptr;
    HDC deviceContext_ = nullptr;
    BITMAPINFO bitmapInfo_ = {};
    POINT lastMousePosition_ = {};
    WINDOWPLACEMENT windowedPlacement_ = {};
    DWORD windowedStyle_ = 0;
    DWORD windowedExStyle_ = 0;
    bool hasLastMousePosition_ = false;
    bool previousFullscreenToggleDown_ = false;
    bool fullscreen_ = false;
#endif
};

} // namespace sr
