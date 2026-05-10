#pragma once

#include "core/Framebuffer.h"

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
    void present(const Framebuffer& framebuffer);

private:
#ifdef _WIN32
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    HWND hwnd_ = nullptr;
    HDC deviceContext_ = nullptr;
    BITMAPINFO bitmapInfo_ = {};
#endif
};

} // namespace sr
