#pragma once

#include "core/Framebuffer.h"
#include "renderer/Vertex.h"

namespace sr {

class TestScene;

class Renderer {
public:
    void render(const TestScene& scene, Framebuffer& framebuffer);

private:
    void draw(const DrawCommand& command, const Mat4& viewProjection, Framebuffer& framebuffer);
    void drawTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& viewProjection, Framebuffer& framebuffer);
};

} // namespace sr
