#include "renderer/Renderer.h"

#include "scenes/TestScene.h"

#include <algorithm>
#include <cmath>

namespace sr {

namespace {

struct ScreenVertex {
    Vec2 position;
    float depth = 0.0f;
    Color color;
};

float edge(Vec2 a, Vec2 b, Vec2 p)
{
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

Color mixColor(Color a, Color b, Color c, float wa, float wb, float wc)
{
    const auto blend = [wa, wb, wc](std::uint8_t av, std::uint8_t bv, std::uint8_t cv) {
        const float value = static_cast<float>(av) * wa + static_cast<float>(bv) * wb + static_cast<float>(cv) * wc;
        return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 255.0f));
    };

    return {
        blend(a.r, b.r, c.r),
        blend(a.g, b.g, c.g),
        blend(a.b, b.b, c.b),
        blend(a.a, b.a, c.a),
    };
}

} // namespace

void Renderer::render(const TestScene& scene, Framebuffer& framebuffer)
{
    framebuffer.clear({ 18, 20, 28, 255 });

    const Mat4 viewProjection = scene.viewProjection(framebuffer.width(), framebuffer.height());
    for (const DrawCommand& command : scene.drawCommands()) {
        draw(command, viewProjection, framebuffer);
    }
}

void Renderer::draw(const DrawCommand& command, const Mat4& viewProjection, Framebuffer& framebuffer)
{
    const Mat4 mvp = viewProjection * command.transform;
    ScreenVertex screen[3] = {};
    const int vertexCount = std::min(command.mesh.vertexCount, 3);

    for (int i = 0; i < vertexCount; ++i) {
        const Vertex& vertex = command.mesh.vertices[i];
        const Vec4 clip = mvp * Vec4 { vertex.position.x, vertex.position.y, vertex.position.z, 1.0f };

        if (clip.w <= 0.000001f) {
            return;
        }

        const float invW = 1.0f / clip.w;
        const float ndcX = clip.x * invW;
        const float ndcY = clip.y * invW;
        const float ndcZ = clip.z * invW;

        screen[i] = {
            {
                (ndcX * 0.5f + 0.5f) * static_cast<float>(framebuffer.width() - 1),
                (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(framebuffer.height() - 1),
            },
            ndcZ,
            vertex.color,
        };
    }

    if (vertexCount != 3) {
        return;
    }

    const Vec2 p0 = screen[0].position;
    const Vec2 p1 = screen[1].position;
    const Vec2 p2 = screen[2].position;
    const float area = edge(p0, p1, p2);

    if (std::abs(area) <= 0.000001f) {
        return;
    }

    const int minX = std::max(0, static_cast<int>(std::floor(std::min({ p0.x, p1.x, p2.x }))));
    const int maxX = std::min(framebuffer.width() - 1, static_cast<int>(std::ceil(std::max({ p0.x, p1.x, p2.x }))));
    const int minY = std::max(0, static_cast<int>(std::floor(std::min({ p0.y, p1.y, p2.y }))));
    const int maxY = std::min(framebuffer.height() - 1, static_cast<int>(std::ceil(std::max({ p0.y, p1.y, p2.y }))));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const Vec2 sample { static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f };
            const float w0 = edge(p1, p2, sample) / area;
            const float w1 = edge(p2, p0, sample) / area;
            const float w2 = edge(p0, p1, sample) / area;

            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) {
                continue;
            }

            const float depth = screen[0].depth * w0 + screen[1].depth * w1 + screen[2].depth * w2;
            const Color color = mixColor(screen[0].color, screen[1].color, screen[2].color, w0, w1, w2);
            framebuffer.setPixelIfCloser(x, y, depth, color);
        }
    }
}

} // namespace sr
