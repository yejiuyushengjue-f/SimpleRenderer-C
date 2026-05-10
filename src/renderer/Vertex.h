#pragma once

#include "core/Color.h"
#include "math/Math.h"

namespace sr {

struct Vertex {
    Vec3 position;
    Color color;
};

struct Mesh {
    const Vertex* vertices = nullptr;
    int vertexCount = 0;
};

struct DrawCommand {
    Mesh mesh;
    Mat4 transform = Mat4::identity();
};

} // namespace sr
