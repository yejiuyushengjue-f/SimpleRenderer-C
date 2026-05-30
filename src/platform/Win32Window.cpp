#include "platform/Win32Window.h"

#include <stdexcept>

namespace sr {

#ifdef _WIN32

namespace {

constexpr const wchar_t* windowClassName = L"SimpleRendererWindowClass";

std::wstring widen(const char* text)
{
    std::wstring result;
    while (*text != '\0') {
        result.push_back(static_cast<unsigned char>(*text));
        ++text;
    }
    return result;
}

} // namespace

Win32Window::Win32Window(void* nativeInstance, int showCommand, int width, int height, const char* title)
{
    HINSTANCE instance = static_cast<HINSTANCE>(nativeInstance);

    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = &Win32Window::windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = windowClassName;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassW(&windowClass);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    const std::wstring wideTitle = widen(title);
    hwnd_ = CreateWindowExW(
        0,
        windowClassName,
        wideTitle.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!hwnd_) {
        throw std::runtime_error("Failed to create Win32 window.");
    }

    deviceContext_ = GetDC(hwnd_);
    if (!deviceContext_) {
        throw std::runtime_error("Failed to acquire Win32 device context.");
    }

    bitmapInfo_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo_.bmiHeader.biWidth = width;
    bitmapInfo_.bmiHeader.biHeight = -height;
    bitmapInfo_.bmiHeader.biPlanes = 1;
    bitmapInfo_.bmiHeader.biBitCount = 32;
    bitmapInfo_.bmiHeader.biCompression = BI_RGB;

    ShowWindow(hwnd_, showCommand);
}

Win32Window::~Win32Window()
{
    if (hwnd_ && deviceContext_) {
        ReleaseDC(hwnd_, deviceContext_);
    }
}

bool Win32Window::processMessages()
{
    MSG message = {};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return true;
}

InputState Win32Window::inputState()
{
    const auto keyDown = [](int key) {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    };

    int renderModeSelection = 0;
    for (int i = 0; i < 7; ++i) {
        if (keyDown('1' + i)) {
            renderModeSelection = i + 1;
        }
    }

    const bool mouseLook = GetForegroundWindow() == hwnd_ && keyDown(VK_RBUTTON);
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;

    POINT mousePosition = {};
    if (GetCursorPos(&mousePosition)) {
        ScreenToClient(hwnd_, &mousePosition);
        if (mouseLook && hasLastMousePosition_) {
            mouseDeltaX = static_cast<float>(mousePosition.x - lastMousePosition_.x);
            mouseDeltaY = static_cast<float>(mousePosition.y - lastMousePosition_.y);
        }
        lastMousePosition_ = mousePosition;
        hasLastMousePosition_ = true;
    } else {
        hasLastMousePosition_ = false;
    }

    return {
        keyDown('W'),
        keyDown('S'),
        keyDown('A'),
        keyDown('D'),
        keyDown('E') || keyDown(VK_SPACE),
        keyDown('Q') || keyDown(VK_CONTROL),
        keyDown(VK_LEFT),
        keyDown(VK_RIGHT),
        keyDown(VK_UP),
        keyDown(VK_DOWN),
        keyDown(VK_SHIFT),
        mouseLook,
        mouseDeltaX,
        mouseDeltaY,
        renderModeSelection,
    };
}

void Win32Window::setTitle(const char* title)
{
    SetWindowTextW(hwnd_, widen(title).c_str());
}

void Win32Window::present(const Framebuffer& framebuffer)
{
    StretchDIBits(
        deviceContext_,
        0,
        0,
        framebuffer.width(),
        framebuffer.height(),
        0,
        0,
        framebuffer.width(),
        framebuffer.height(),
        framebuffer.pixels(),
        &bitmapInfo_,
        DIB_RGB_COLORS,
        SRCCOPY);
}

LRESULT CALLBACK Win32Window::windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }
}

#else

Win32Window::Win32Window(void*, int, int, int, const char*)
{
    throw std::runtime_error("SimpleRenderer currently supports Windows only.");
}

Win32Window::~Win32Window() = default;

bool Win32Window::processMessages()
{
    return false;
}

InputState Win32Window::inputState()
{
    return {};
}

void Win32Window::setTitle(const char*)
{
}

void Win32Window::present(const Framebuffer&)
{
}

#endif

} // namespace sr
