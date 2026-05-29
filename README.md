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
- Wavefront OBJ model loading with support for positions, UVs, normals, negative indices, and polygon triangulation.
- Triangle rasterization using screen-space bounding boxes and barycentric coordinates.
- Vertex attributes: position, UV, normal, and vertex color.
- Texture mapping with JPEG loading through Windows Imaging Component.
- Generated checkerboard fallback textures when image files cannot be found or decoded.
- Perspective-correct interpolation for UVs, normals, and view-space position.
- Pixel shading with texture color multiplied by vertex color.
- Multi-light shading with ambient light, multiple directional lights, point lights, Lambert diffuse, and Blinn-Phong specular highlights.
- Shadow mapping for the primary directional light using a CPU-generated light-space depth map.
- Procedural 3D mesh generation for a UV sphere and a textured cube.
- Test scene that loads the first OBJ model found under `res/Model` or `res/Models`, faces it toward the initial camera view, and keeps it rotating horizontally; if none is found, it falls back to a textured sphere in front of a textured cube.

## Assets

The default scene looks for these texture files:

- `res/Texture/Frosted Metal Texture.jpeg`
- `res/Texture/Brushed metal texture.jpeg`

The renderer checks the current working directory and common parent paths used by build output folders. If an image exists but cannot be decoded, the scene falls back to a generated checkerboard texture instead of aborting startup.

OBJ models can be placed in either of these folders:

- `res/Model`
- `res/Models`

The loader reads the first `.obj` file it finds, normalizes it to fit the scene, and uses the frosted metal texture as its material texture. Supported face formats include `v`, `v/vt`, `v//vn`, and `v/vt/vn`.

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

For interactive testing, prefer the optimized Release build:

```powershell
cmake --build build --config Release
.\build\Release\SimpleRenderer.exe
```

The renderer is CPU-only. High-polygon OBJ files and shadow mapping are expensive in Debug builds, so use the Release build for smoother navigation with complex models.
