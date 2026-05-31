#pragma once

#include "core/Color.h"
#include "math/Math.h"
#include "renderer/Material.h"

namespace sr {

struct Vertex {
    Vec3 position;
    Vec2 uv;
    Vec3 normal;
    Color color;
    Vec3 tangent = { 1.0f, 0.0f, 0.0f };
    float tangentSign = 1.0f;
};

struct Mesh {
    const Vertex* vertices = nullptr;
    int vertexCount = 0;
};

struct DrawCommand {
    Mesh mesh;
    Material material;
    bool castsShadow = true;
    Mat4 transform = Mat4::identity();
};

inline Vec3 fallbackTangent(Vec3 normal)
{
    const Vec3 n = normalize(normal);
    const Vec3 axis = std::abs(n.y) < 0.999f ? Vec3 { 0.0f, 1.0f, 0.0f } : Vec3 { 1.0f, 0.0f, 0.0f };
    return normalize(cross(axis, n));
}

inline Vec3 orthogonalizeTangent(Vec3 tangent, Vec3 normal)
{
    const Vec3 n = normalize(normal);
    const Vec3 t = tangent - n * dot(n, tangent);
    if (dot(t, t) <= 0.000001f) {
        return fallbackTangent(n);
    }

    return normalize(t);
}

inline void assignTriangleTangents(Vertex& v0, Vertex& v1, Vertex& v2)
{
    const Vec3 edge1 = v1.position - v0.position;
    const Vec3 edge2 = v2.position - v0.position;
    const Vec2 deltaUv1 { v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y };
    const Vec2 deltaUv2 { v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y };
    const float determinant = deltaUv1.x * deltaUv2.y - deltaUv1.y * deltaUv2.x;

    Vec3 tangent = fallbackTangent(v0.normal + v1.normal + v2.normal);
    float tangentSign = 1.0f;
    if (std::abs(determinant) > 0.000001f) {
        const float inverse = 1.0f / determinant;
        tangent = (edge1 * deltaUv2.y - edge2 * deltaUv1.y) * inverse;
        tangentSign = determinant < 0.0f ? -1.0f : 1.0f;
    }

    v0.tangent = orthogonalizeTangent(tangent, v0.normal);
    v1.tangent = orthogonalizeTangent(tangent, v1.normal);
    v2.tangent = orthogonalizeTangent(tangent, v2.normal);
    v0.tangentSign = tangentSign;
    v1.tangentSign = tangentSign;
    v2.tangentSign = tangentSign;
}

inline void assignMeshTangents(Vertex* vertices, int vertexCount)
{
    if (!vertices) {
        return;
    }

    for (int i = 0; i + 2 < vertexCount; i += 3) {
        assignTriangleTangents(vertices[i], vertices[i + 1], vertices[i + 2]);
    }
}

} // namespace sr
