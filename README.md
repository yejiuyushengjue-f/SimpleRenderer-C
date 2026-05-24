# SimpleRenderer

A small software renderer for learning the classic CPU rendering pipeline.

## Current Features

- Win32 window and CPU-side BGRA framebuffer presentation.
- Color buffer and depth buffer with per-pixel depth testing.
- Basic math library: `Vec2`, `Vec3`, `Vec4`, `Mat4`, dot/cross/normalize, translation, X/Y/Z rotation, and perspective projection.
- Main application loop with scene update, rendering, and framebuffer presentation.
- Camera module with keyboard-controlled movement and view/projection matrix generation.
- Draw-command based pipeline with mesh vertices, model transform, and optional texture binding.
- Triangle-list mesh rendering; each mesh can contain many triangles.
- Triangle rasterization using screen-space bounding boxes and barycentric coordinates.
- Vertex attributes: position, UV, normal, and vertex color.
- Texture mapping with JPEG loading through Windows Imaging Component.
- Generated checkerboard fallback textures when image files cannot be found or decoded.
- Perspective-correct interpolation for UVs, normals, and view-space position.
- Pixel shading with texture color multiplied by vertex color.
- Lighting model with ambient light, directional Lambert diffuse, and Blinn-Phong specular highlights.
- Procedural 3D mesh generation for a UV sphere and a textured cube.
- Test scene with a textured sphere in front of a textured cube to demonstrate depth, perspective correction, texture mapping, and lighting.

## Assets

The default scene looks for these texture files:

- `res/Texture/Frosted Metal Texture.jpeg`
- `res/Texture/Brushed metal texture.jpeg`

The renderer checks the current working directory and common parent paths used by build output folders. If an image exists but cannot be decoded, the scene falls back to a generated checkerboard texture instead of aborting startup.

## Controls

- `W` / `S`: move forward and backward.
- `A` / `D`: move left and right.
- `Q` / `E`: move down and up.
- `Space`: move up.
- Arrow keys: look around.
- `Shift`: hold to move faster.

## Build

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\SimpleRenderer.exe
```

If your generator creates a different config folder, use the path printed by CMake.
