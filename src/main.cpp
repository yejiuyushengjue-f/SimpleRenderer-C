#include "core/Application.h"

#include <stdexcept>

#ifdef _WIN32
#include <windows.h>

// Windows 系统：使用 wWinMain，获取 Windows 资源
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    sr::Application app(instance, showCommand);  // 传递 Windows 特定数据
    return app.run();
}
#else
// 其他系统：使用标准 main
int main()
{
    sr::Application app(nullptr, 0);  // nullptr 和 0 代替 Windows 参数
    return app.run();
}
#endif
