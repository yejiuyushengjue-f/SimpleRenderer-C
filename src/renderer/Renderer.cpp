#include "renderer/Renderer.h"

#include "scenes/TestScene.h"
#include "renderer/Texture.h"

#include <algorithm>
#include <cmath>

namespace sr {

namespace {

struct ScreenVertex {
    Vec2 position;
    float depth = 0.0f;
    float invW = 1.0f;
    Vec3 viewPositionOverW;
    Vec2 uvOverW;
    Vec3 normalOverW;
    Color color;
};

struct DirectionalLight {
    Vec3 direction;
    Color color;
    float intensity = 1.0f;
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

Color modulate(Color a, Color b)
{
    return {
        static_cast<std::uint8_t>((static_cast<int>(a.r) * static_cast<int>(b.r)) / 255),
        static_cast<std::uint8_t>((static_cast<int>(a.g) * static_cast<int>(b.g)) / 255),
        static_cast<std::uint8_t>((static_cast<int>(a.b) * static_cast<int>(b.b)) / 255),
        static_cast<std::uint8_t>((static_cast<int>(a.a) * static_cast<int>(b.a)) / 255),
    };
}

Color scaleColor(Color color, float scale)
{
    const auto scaleChannel = [scale](std::uint8_t value) {
        return static_cast<std::uint8_t>(std::clamp(static_cast<float>(value) * scale, 0.0f, 255.0f));
    };

    return {
        scaleChannel(color.r),
        scaleChannel(color.g),
        scaleChannel(color.b),
        color.a,
    };
}

Color applyLighting(Color albedo, Vec3 normal, Vec3 viewPosition, const DirectionalLight& light, float ambient, float specularStrength, float shininess)
{
    const Vec3 n = normalize(normal);
    const Vec3 lightToSurface = normalize(light.direction);
    const Vec3 viewDirection = normalize({ -viewPosition.x, -viewPosition.y, -viewPosition.z });
    const Vec3 halfVector = normalize(lightToSurface + viewDirection);

    const float diffuse = std::max(0.0f, dot(n, lightToSurface)) * light.intensity;
    const float specular = std::pow(std::max(0.0f, dot(n, halfVector)), shininess) * specularStrength * light.intensity;

    const Color litDiffuse = modulate(scaleColor(albedo, ambient + diffuse), light.color);
    return {
        static_cast<std::uint8_t>(std::clamp(static_cast<float>(litDiffuse.r) + static_cast<float>(light.color.r) * specular, 0.0f, 255.0f)),
        static_cast<std::uint8_t>(std::clamp(static_cast<float>(litDiffuse.g) + static_cast<float>(light.color.g) * specular, 0.0f, 255.0f)),
        static_cast<std::uint8_t>(std::clamp(static_cast<float>(litDiffuse.b) + static_cast<float>(light.color.b) * specular, 0.0f, 255.0f)),
        albedo.a,
    };
}

Vec3 transformDirection(const Mat4& matrix, Vec3 direction)
{
    const Vec4 transformed = matrix * Vec4 { direction.x, direction.y, direction.z, 0.0f };
    return normalize({ transformed.x, transformed.y, transformed.z });
}

} // namespace

void Renderer::render(const TestScene& scene, const Camera& camera, Framebuffer& framebuffer)
{
    framebuffer.clear({ 18, 20, 28, 255 });

    const Mat4 view = camera.viewMatrix();
    const Mat4 projection = camera.projectionMatrix(framebuffer.width(), framebuffer.height());
    for (const DrawCommand& command : scene.drawCommands()) {
        draw(command, view, projection, framebuffer);
    }
}

void Renderer::draw(const DrawCommand& command, const Mat4& view, const Mat4& projection, Framebuffer& framebuffer)
{
    if (!command.mesh.vertices || command.mesh.vertexCount < 3) {
        return;
    }

    for (int i = 0; i + 2 < command.mesh.vertexCount; i += 3) {
        drawTriangle(command, command.mesh.vertices + i, view, projection, framebuffer);
    }
}

void Renderer::drawTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& view, const Mat4& projection, Framebuffer& framebuffer)
{
    const Mat4 modelView = view * command.transform;
    const Mat4 mvp = projection * modelView;
    const DirectionalLight light {
        normalize({ -0.45f, -0.55f, 1.0f }),
        { 255, 244, 224, 255 },
        0.8f,
    };
    constexpr float ambient = 0.24f;
    constexpr float specularStrength = 0.48f;
    constexpr float shininess = 48.0f;

    ScreenVertex screen[3] = {};

    for (int i = 0; i < 3; ++i) {
        const Vertex& vertex = vertices[i];
        const Vec4 viewPosition = modelView * Vec4 { vertex.position.x, vertex.position.y, vertex.position.z, 1.0f };
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
            invW,
            { viewPosition.x * invW, viewPosition.y * invW, viewPosition.z * invW },
            { vertex.uv.x * invW, vertex.uv.y * invW },
            transformDirection(modelView, vertex.normal) * invW,
            vertex.color,
        };
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
            const float interpolatedInvW = screen[0].invW * w0 + screen[1].invW * w1 + screen[2].invW * w2;
            if (interpolatedInvW <= 0.000001f) {
                continue;
            }

            const Vec2 uv {
                (screen[0].uvOverW.x * w0 + screen[1].uvOverW.x * w1 + screen[2].uvOverW.x * w2) / interpolatedInvW,
                (screen[0].uvOverW.y * w0 + screen[1].uvOverW.y * w1 + screen[2].uvOverW.y * w2) / interpolatedInvW,
            };
            const Vec3 viewPosition = {
                (screen[0].viewPositionOverW.x * w0 + screen[1].viewPositionOverW.x * w1 + screen[2].viewPositionOverW.x * w2) / interpolatedInvW,
                (screen[0].viewPositionOverW.y * w0 + screen[1].viewPositionOverW.y * w1 + screen[2].viewPositionOverW.y * w2) / interpolatedInvW,
                (screen[0].viewPositionOverW.z * w0 + screen[1].viewPositionOverW.z * w1 + screen[2].viewPositionOverW.z * w2) / interpolatedInvW,
            };
            const Vec3 normal = {
                (screen[0].normalOverW.x * w0 + screen[1].normalOverW.x * w1 + screen[2].normalOverW.x * w2) / interpolatedInvW,
                (screen[0].normalOverW.y * w0 + screen[1].normalOverW.y * w1 + screen[2].normalOverW.y * w2) / interpolatedInvW,
                (screen[0].normalOverW.z * w0 + screen[1].normalOverW.z * w1 + screen[2].normalOverW.z * w2) / interpolatedInvW,
            };

            Color color = mixColor(screen[0].color, screen[1].color, screen[2].color, w0, w1, w2);
            if (command.texture) {
                color = modulate(command.texture->sample(uv), color);
            }
            color = applyLighting(color, normal, viewPosition, light, ambient, specularStrength, shininess);

            framebuffer.setPixelIfCloser(x, y, depth, color);
        }
    }
}

} // namespace sr
