#pragma once

#include "core/Color.h"
#include "math/Math.h"

namespace sr {

class Texture;

struct Vertex {
    Vec3 position;
    Vec2 uv;
    Vec3 normal;
    Color color;
};

struct Mesh {
    const Vertex* vertices = nullptr;
    int vertexCount = 0;
};

struct DrawCommand {
    Mesh mesh;
    const Texture* texture = nullptr;
    bool castsShadow = true;
    Mat4 transform = Mat4::identity();
};

} // namespace sr
