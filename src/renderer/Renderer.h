#pragma once

#include "core/Camera.h"
#include "core/Framebuffer.h"
#include "renderer/Vertex.h"

#include <array>
#include <vector>

namespace sr {

class TestScene;

enum class RenderMode {
    Final = 0,
    Albedo,
    Normal,
    Depth,
    UV,
    Shadow,
    Light,
    LightDepth,
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
    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;
    const char* renderModeName() const;

private:
    ShadowMap shadowMap_;
    RenderMode renderMode_ = RenderMode::Final;

    void draw(
        const DrawCommand& command,
        const Mat4& view,
        const Mat4& projection,
        const Mat4& lightViewProjection,
        const DirectionalLight& light,
        const ViewLightSet& lights,
        const ShadowMap& shadowMap,
        Framebuffer& framebuffer);
    void drawTriangle(
        const DrawCommand& command,
        const Vertex* vertices,
        const Mat4& view,
        const Mat4& projection,
        const Mat4& lightViewProjection,
        const DirectionalLight& light,
        const ViewLightSet& lights,
        const ShadowMap& shadowMap,
        Framebuffer& framebuffer);
    void renderShadowMap(const TestScene& scene, const Mat4& lightViewProjection, ShadowMap& shadowMap);
    void drawShadowTriangle(const DrawCommand& command, const Vertex* vertices, const Mat4& lightViewProjection, ShadowMap& shadowMap);
};

} // namespace sr
