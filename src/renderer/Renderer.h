#pragma once

#include "core/Camera.h"
#include "core/Framebuffer.h"
#include "renderer/Vertex.h"

#include <vector>

namespace sr {

class TestScene;

struct ShadowMap {
    int width = 512;
    int height = 512;
    std::vector<float> depth;

    ShadowMap(int width = 512, int height = 512);
    void clear();
    bool setIfCloser(int x, int y, float value);
    float sample(int x, int y) const;
};

class Renderer {
public:
    void render(const TestScene& scene, const Camera& camera, Framebuffer& framebuffer);

private:
    ShadowMap shadowMap_;

    void draw(const DrawCommand& command, const Mat4& view, const Mat4& projection, const Mat4& lightViewProjection, const ShadowMap& shadowMap, Framebuffer& framebuffer);
    void drawTriangle(
        const DrawCommand& command,
        const Vertex* vertices,
        const Mat4& view,
        const Mat4& projection,
        const Mat4& lightViewProjection,
        const ShadowMap& shadowMap,
        Framebuffer& framebuffer);
    void renderShadowMap(const TestScene& scene, const Mat4& lightViewProjection, ShadowMap& shadowMap);
    void drawShadowTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& lightViewProjection, ShadowMap& shadowMap);
};

} // namespace sr
