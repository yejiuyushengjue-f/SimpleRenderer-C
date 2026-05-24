#pragma once

#include "core/Camera.h"
#include "core/Framebuffer.h"
#include "renderer/Vertex.h"

namespace sr {

class TestScene;

class Renderer {
public:
    void render(const TestScene& scene, const Camera& camera, Framebuffer& framebuffer);

private:
    void draw(const DrawCommand& command, const Mat4& view, const Mat4& projection, Framebuffer& framebuffer);
    void drawTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& view, const Mat4& projection, Framebuffer& framebuffer);
};

} // namespace sr
