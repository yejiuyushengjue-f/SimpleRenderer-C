#include "renderer/Renderer.h"

#include "scenes/TestScene.h"
#include "renderer/Texture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace sr {

namespace {

struct ScreenVertex {
    Vec2 position;
    float depth = 0.0f;
    float invW = 1.0f;
    Vec3 worldPositionOverW;
    Vec3 viewPositionOverW;
    Vec2 uvOverW;
    Vec3 normalOverW;
    Color color;
};

struct ClipVertex {
    Vec4 clip;
    Vec3 worldPosition;
    Vec3 viewPosition;
    Vec2 uv;
    Vec3 normal;
    Color color;
};

struct ClipPolygon {
    std::array<ClipVertex, 12> vertices;
    int count = 0;
};

struct ShadowVertex {
    Vec2 position;
    float depth = 0.0f;
};

struct DirectionalLight {
    Vec3 direction;
    Color color;
    float intensity = 1.0f;
};

struct PointLight {
    Vec3 position;
    Color color;
    float intensity = 1.0f;
    float range = 1.0f;
};

struct ViewLightSet {
    std::array<DirectionalLight, 3> directionalLights;
    std::array<PointLight, 2> pointLights;
};

float edge(Vec2 a, Vec2 b, Vec2 p)
{
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

float clipDistance(const ClipVertex& vertex, int plane)
{
    switch (plane) {
    case 0:
        return vertex.clip.x + vertex.clip.w;
    case 1:
        return vertex.clip.w - vertex.clip.x;
    case 2:
        return vertex.clip.y + vertex.clip.w;
    case 3:
        return vertex.clip.w - vertex.clip.y;
    case 4:
        return vertex.clip.z + vertex.clip.w;
    default:
        return vertex.clip.w - vertex.clip.z;
    }
}

Color lerpColor(Color a, Color b, float t)
{
    const auto lerpChannel = [t](std::uint8_t av, std::uint8_t bv) {
        return static_cast<std::uint8_t>(std::clamp(
            static_cast<float>(av) + (static_cast<float>(bv) - static_cast<float>(av)) * t,
            0.0f,
            255.0f));
    };

    return {
        lerpChannel(a.r, b.r),
        lerpChannel(a.g, b.g),
        lerpChannel(a.b, b.b),
        lerpChannel(a.a, b.a),
    };
}

ClipVertex lerpClipVertex(const ClipVertex& a, const ClipVertex& b, float t)
{
    const auto lerpFloat = [t](float av, float bv) {
        return av + (bv - av) * t;
    };

    return {
        {
            lerpFloat(a.clip.x, b.clip.x),
            lerpFloat(a.clip.y, b.clip.y),
            lerpFloat(a.clip.z, b.clip.z),
            lerpFloat(a.clip.w, b.clip.w),
        },
        {
            lerpFloat(a.worldPosition.x, b.worldPosition.x),
            lerpFloat(a.worldPosition.y, b.worldPosition.y),
            lerpFloat(a.worldPosition.z, b.worldPosition.z),
        },
        {
            lerpFloat(a.viewPosition.x, b.viewPosition.x),
            lerpFloat(a.viewPosition.y, b.viewPosition.y),
            lerpFloat(a.viewPosition.z, b.viewPosition.z),
        },
        {
            lerpFloat(a.uv.x, b.uv.x),
            lerpFloat(a.uv.y, b.uv.y),
        },
        normalize({
            lerpFloat(a.normal.x, b.normal.x),
            lerpFloat(a.normal.y, b.normal.y),
            lerpFloat(a.normal.z, b.normal.z),
        }),
        lerpColor(a.color, b.color, t),
    };
}

ClipPolygon clipPolygonAgainstPlane(const ClipPolygon& input, int plane)
{
    ClipPolygon output;
    if (input.count == 0) {
        return output;
    }

    ClipVertex previous = input.vertices[static_cast<std::size_t>(input.count - 1)];
    float previousDistance = clipDistance(previous, plane);
    bool previousInside = previousDistance >= 0.0f;

    for (int i = 0; i < input.count; ++i) {
        const ClipVertex& current = input.vertices[static_cast<std::size_t>(i)];
        const float currentDistance = clipDistance(current, plane);
        const bool currentInside = currentDistance >= 0.0f;

        if (currentInside != previousInside) {
            const float denominator = previousDistance - currentDistance;
            const float t = std::abs(denominator) <= 0.000001f ? 0.0f : previousDistance / denominator;
            if (output.count < static_cast<int>(output.vertices.size())) {
                output.vertices[static_cast<std::size_t>(output.count++)] = lerpClipVertex(previous, current, t);
            }
        }

        if (currentInside && output.count < static_cast<int>(output.vertices.size())) {
            output.vertices[static_cast<std::size_t>(output.count++)] = current;
        }

        previous = current;
        previousDistance = currentDistance;
        previousInside = currentInside;
    }

    return output;
}

ClipPolygon clipTriangleToFrustum(const ClipVertex* triangle)
{
    ClipPolygon polygon;
    polygon.vertices[0] = triangle[0];
    polygon.vertices[1] = triangle[1];
    polygon.vertices[2] = triangle[2];
    polygon.count = 3;

    for (int plane = 0; plane < 6 && polygon.count > 0; ++plane) {
        polygon = clipPolygonAgainstPlane(polygon, plane);
    }
    return polygon;
}

bool toScreenVertex(const ClipVertex& vertex, int width, int height, ScreenVertex& out)
{
    if (std::abs(vertex.clip.w) <= 0.000001f) {
        return false;
    }

    const float invW = 1.0f / vertex.clip.w;
    const float ndcX = vertex.clip.x * invW;
    const float ndcY = vertex.clip.y * invW;
    const float ndcZ = vertex.clip.z * invW;

    out = {
        {
            (ndcX * 0.5f + 0.5f) * static_cast<float>(width - 1),
            (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(height - 1),
        },
        ndcZ,
        invW,
        vertex.worldPosition * invW,
        vertex.viewPosition * invW,
        { vertex.uv.x * invW, vertex.uv.y * invW },
        vertex.normal * invW,
        vertex.color,
    };
    return true;
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

Color addLight(Color current, Color lightColor, float diffuse, float specular, Color albedo)
{
    return {
        static_cast<std::uint8_t>(std::clamp(
            static_cast<float>(current.r)
                + static_cast<float>(albedo.r) * (static_cast<float>(lightColor.r) / 255.0f) * diffuse
                + static_cast<float>(lightColor.r) * specular,
            0.0f,
            255.0f)),
        static_cast<std::uint8_t>(std::clamp(
            static_cast<float>(current.g)
                + static_cast<float>(albedo.g) * (static_cast<float>(lightColor.g) / 255.0f) * diffuse
                + static_cast<float>(lightColor.g) * specular,
            0.0f,
            255.0f)),
        static_cast<std::uint8_t>(std::clamp(
            static_cast<float>(current.b)
                + static_cast<float>(albedo.b) * (static_cast<float>(lightColor.b) / 255.0f) * diffuse
                + static_cast<float>(lightColor.b) * specular,
            0.0f,
            255.0f)),
        albedo.a,
    };
}

Color applyLighting(
    Color albedo,
    Vec3 normal,
    Vec3 viewPosition,
    const ViewLightSet& lights,
    float ambient,
    float specularStrength,
    float shininess,
    float primaryShadow)
{
    const Vec3 n = normalize(normal);
    const Vec3 viewDirection = normalize({ -viewPosition.x, -viewPosition.y, -viewPosition.z });

    Color result {
        static_cast<std::uint8_t>(std::clamp(static_cast<float>(albedo.r) * ambient, 0.0f, 255.0f)),
        static_cast<std::uint8_t>(std::clamp(static_cast<float>(albedo.g) * ambient, 0.0f, 255.0f)),
        static_cast<std::uint8_t>(std::clamp(static_cast<float>(albedo.b) * ambient, 0.0f, 255.0f)),
        albedo.a,
    };

    for (std::size_t i = 0; i < lights.directionalLights.size(); ++i) {
        const DirectionalLight& light = lights.directionalLights[i];
        const float shadow = i == 0 ? primaryShadow : 1.0f;
        const Vec3 lightToSurface = normalize(light.direction);
        const Vec3 halfVector = normalize(lightToSurface + viewDirection);
        const float diffuse = std::max(0.0f, dot(n, lightToSurface)) * light.intensity * shadow;
        const float specular = std::pow(std::max(0.0f, dot(n, halfVector)), shininess) * specularStrength * light.intensity * shadow;
        result = addLight(result, light.color, diffuse, specular, albedo);
    }

    for (const PointLight& light : lights.pointLights) {
        const Vec3 toLight = light.position - viewPosition;
        const float distanceSquared = std::max(0.0001f, dot(toLight, toLight));
        const float distance = std::sqrt(distanceSquared);
        const float attenuation = std::clamp(1.0f - distance / light.range, 0.0f, 1.0f);
        if (attenuation <= 0.0f) {
            continue;
        }

        const Vec3 lightToSurface = toLight / distance;
        const Vec3 halfVector = normalize(lightToSurface + viewDirection);
        const float diffuse = std::max(0.0f, dot(n, lightToSurface)) * light.intensity * attenuation * attenuation;
        const float specular = std::pow(std::max(0.0f, dot(n, halfVector)), shininess) * specularStrength * light.intensity * attenuation * attenuation;
        result = addLight(result, light.color, diffuse, specular, albedo);
    }

    return result;
}

Vec3 transformDirection(const Mat4& matrix, Vec3 direction)
{
    const Vec4 transformed = matrix * Vec4 { direction.x, direction.y, direction.z, 0.0f };
    return normalize({ transformed.x, transformed.y, transformed.z });
}

DirectionalLight sceneLight()
{
    return {
        normalize({ -0.45f, -0.55f, 1.0f }),
        { 255, 244, 224, 255 },
        0.8f,
    };
}

ViewLightSet sceneLightsInView(const Mat4& view)
{
    const DirectionalLight primary = sceneLight();

    return {
        {
            DirectionalLight { transformDirection(view, primary.direction), primary.color, primary.intensity },
            DirectionalLight { transformDirection(view, normalize({ 0.75f, 0.35f, 0.25f })), { 135, 178, 255, 255 }, 0.28f },
            DirectionalLight { transformDirection(view, normalize({ -0.2f, 0.85f, -0.45f })), { 255, 145, 112, 255 }, 0.18f },
        },
        {
            PointLight {
                { (view * Vec4 { -1.35f, 0.85f, -1.7f, 1.0f }).x, (view * Vec4 { -1.35f, 0.85f, -1.7f, 1.0f }).y, (view * Vec4 { -1.35f, 0.85f, -1.7f, 1.0f }).z },
                { 255, 205, 150, 255 },
                0.75f,
                3.0f,
            },
            PointLight {
                { (view * Vec4 { 1.45f, -0.35f, -2.25f, 1.0f }).x, (view * Vec4 { 1.45f, -0.35f, -2.25f, 1.0f }).y, (view * Vec4 { 1.45f, -0.35f, -2.25f, 1.0f }).z },
                { 120, 210, 255, 255 },
                0.55f,
                2.4f,
            },
        },
    };
}

Mat4 sceneLightViewProjection(const DirectionalLight& light)
{
    const Vec3 lightPosition = light.direction * 6.5f;
    const Mat4 lightView = Mat4::lookAt(lightPosition, { 0.0f, 0.0f, -3.0f }, { 0.0f, 1.0f, 0.0f });
    const Mat4 lightProjection = Mat4::orthographic(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 12.0f);
    return lightProjection * lightView;
}

float shadowFactor(Vec3 worldPosition, Vec3 normal, const DirectionalLight& light, const Mat4& lightViewProjection, const ShadowMap& shadowMap)
{
    const Vec4 lightClip = lightViewProjection * Vec4 { worldPosition.x, worldPosition.y, worldPosition.z, 1.0f };
    if (std::abs(lightClip.w) <= 0.000001f) {
        return 1.0f;
    }

    const float invW = 1.0f / lightClip.w;
    const float ndcX = lightClip.x * invW;
    const float ndcY = lightClip.y * invW;
    const float ndcZ = lightClip.z * invW;
    if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f || ndcZ < -1.0f || ndcZ > 1.0f) {
        return 1.0f;
    }

    const float sx = (ndcX * 0.5f + 0.5f) * static_cast<float>(shadowMap.width - 1);
    const float sy = (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(shadowMap.height - 1);
    const int ix = static_cast<int>(std::round(sx));
    const int iy = static_cast<int>(std::round(sy));

    const float slopeBias = 0.01f * (1.0f - std::max(0.0f, dot(normalize(normal), normalize(light.direction))));
    const float bias = std::max(0.0035f, slopeBias);
    const float closestDepth = shadowMap.sample(ix, iy);
    return ndcZ > closestDepth + bias ? 0.35f : 1.0f;
}

} // namespace

ShadowMap::ShadowMap(int mapWidth, int mapHeight)
    : width(mapWidth)
    , height(mapHeight)
    , depth(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), std::numeric_limits<float>::infinity())
{
}

void ShadowMap::clear()
{
    std::fill(depth.begin(), depth.end(), std::numeric_limits<float>::infinity());
}

bool ShadowMap::setIfCloser(int x, int y, float value)
{
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return false;
    }

    const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x);
    if (value >= depth[index]) {
        return false;
    }

    depth[index] = value;
    return true;
}

float ShadowMap::sample(int x, int y) const
{
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return std::numeric_limits<float>::infinity();
    }

    return depth[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
}

void Renderer::render(const TestScene& scene, const Camera& camera, Framebuffer& framebuffer)
{
    framebuffer.clear({ 18, 20, 28, 255 });

    const DirectionalLight light = sceneLight();
    const Mat4 lightViewProjection = sceneLightViewProjection(light);
    renderShadowMap(scene, lightViewProjection, shadowMap_);

    const Mat4 view = camera.viewMatrix();
    const Mat4 projection = camera.projectionMatrix(framebuffer.width(), framebuffer.height());
    for (const DrawCommand& command : scene.drawCommands()) {
        draw(command, view, projection, lightViewProjection, shadowMap_, framebuffer);
    }
}

void Renderer::draw(const DrawCommand& command, const Mat4& view, const Mat4& projection, const Mat4& lightViewProjection, const ShadowMap& shadowMap, Framebuffer& framebuffer)
{
    if (!command.mesh.vertices || command.mesh.vertexCount < 3) {
        return;
    }

    for (int i = 0; i + 2 < command.mesh.vertexCount; i += 3) {
        drawTriangle(command, command.mesh.vertices + i, view, projection, lightViewProjection, shadowMap, framebuffer);
    }
}

void Renderer::drawTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& view, const Mat4& projection, const Mat4& lightViewProjection, const ShadowMap& shadowMap, Framebuffer& framebuffer)
{
    const Mat4 modelView = view * command.transform;
    const Mat4 mvp = projection * modelView;
    const DirectionalLight light = sceneLight();
    const ViewLightSet lights = sceneLightsInView(view);
    constexpr float ambient = 0.24f;
    constexpr float specularStrength = 0.48f;
    constexpr float shininess = 48.0f;

    ClipVertex clipTriangle[3] = {};

    for (int i = 0; i < 3; ++i) {
        const Vertex& vertex = vertices[i];
        const Vec4 worldPosition = command.transform * Vec4 { vertex.position.x, vertex.position.y, vertex.position.z, 1.0f };
        const Vec4 viewPosition = modelView * Vec4 { vertex.position.x, vertex.position.y, vertex.position.z, 1.0f };
        const Vec4 clip = mvp * Vec4 { vertex.position.x, vertex.position.y, vertex.position.z, 1.0f };

        clipTriangle[i] = {
            clip,
            { worldPosition.x, worldPosition.y, worldPosition.z },
            { viewPosition.x, viewPosition.y, viewPosition.z },
            vertex.uv,
            transformDirection(modelView, vertex.normal),
            vertex.color,
        };
    }

    const ClipPolygon clippedPolygon = clipTriangleToFrustum(clipTriangle);
    if (clippedPolygon.count < 3) {
        return;
    }

    for (int i = 1; i + 1 < clippedPolygon.count; ++i) {
        ScreenVertex screen[3] = {};
        if (!toScreenVertex(clippedPolygon.vertices[0], framebuffer.width(), framebuffer.height(), screen[0])
            || !toScreenVertex(clippedPolygon.vertices[static_cast<std::size_t>(i)], framebuffer.width(), framebuffer.height(), screen[1])
            || !toScreenVertex(clippedPolygon.vertices[static_cast<std::size_t>(i + 1)], framebuffer.width(), framebuffer.height(), screen[2])) {
            continue;
        }

        const Vec2 p0 = screen[0].position;
        const Vec2 p1 = screen[1].position;
        const Vec2 p2 = screen[2].position;
        const float area = edge(p0, p1, p2);

        if (area >= -0.000001f) {
            continue;
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
                const Vec3 worldPosition = {
                    (screen[0].worldPositionOverW.x * w0 + screen[1].worldPositionOverW.x * w1 + screen[2].worldPositionOverW.x * w2) / interpolatedInvW,
                    (screen[0].worldPositionOverW.y * w0 + screen[1].worldPositionOverW.y * w1 + screen[2].worldPositionOverW.y * w2) / interpolatedInvW,
                    (screen[0].worldPositionOverW.z * w0 + screen[1].worldPositionOverW.z * w1 + screen[2].worldPositionOverW.z * w2) / interpolatedInvW,
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
                const float shadow = shadowFactor(worldPosition, normal, light, lightViewProjection, shadowMap);
                color = applyLighting(color, normal, viewPosition, lights, ambient, specularStrength, shininess, shadow);

                framebuffer.setPixelIfCloser(x, y, depth, color);
            }
        }
    }
}

void Renderer::renderShadowMap(const TestScene& scene, const Mat4& lightViewProjection, ShadowMap& shadowMap)
{
    shadowMap.clear();
    for (const DrawCommand& command : scene.drawCommands()) {
        if (!command.castsShadow || !command.mesh.vertices || command.mesh.vertexCount < 3) {
            continue;
        }

        for (int i = 0; i + 2 < command.mesh.vertexCount; i += 3) {
            drawShadowTriangle(command, command.mesh.vertices + i, lightViewProjection, shadowMap);
        }
    }
}

void Renderer::drawShadowTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& lightViewProjection, ShadowMap& shadowMap)
{
    const Mat4 lightMvp = lightViewProjection * command.transform;
    ShadowVertex screen[3] = {};

    for (int i = 0; i < 3; ++i) {
        const Vertex& vertex = vertices[i];
        const Vec4 clip = lightMvp * Vec4 { vertex.position.x, vertex.position.y, vertex.position.z, 1.0f };
        if (std::abs(clip.w) <= 0.000001f) {
            return;
        }

        const float invW = 1.0f / clip.w;
        const float ndcX = clip.x * invW;
        const float ndcY = clip.y * invW;
        const float ndcZ = clip.z * invW;

        screen[i] = {
            {
                (ndcX * 0.5f + 0.5f) * static_cast<float>(shadowMap.width - 1),
                (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(shadowMap.height - 1),
            },
            ndcZ,
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
    const int maxX = std::min(shadowMap.width - 1, static_cast<int>(std::ceil(std::max({ p0.x, p1.x, p2.x }))));
    const int minY = std::max(0, static_cast<int>(std::floor(std::min({ p0.y, p1.y, p2.y }))));
    const int maxY = std::min(shadowMap.height - 1, static_cast<int>(std::ceil(std::max({ p0.y, p1.y, p2.y }))));

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
            shadowMap.setIfCloser(x, y, depth);
        }
    }
}

} // namespace sr
