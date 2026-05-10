# SimpleRenderer

A small software renderer skeleton for learning:

- Win32 window and CPU pixel framebuffer
- Basic math library (`Vec2`, `Vec3`, `Vec4`, `Mat4`, colors)
- Main loop and renderer pipeline shape
- A simple rotating triangle test scene

## Build

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\SimpleRenderer.exe
```

If your generator creates a different config folder, use the path printed by CMake.
