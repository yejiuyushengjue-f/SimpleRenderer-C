#include "scenes/TestScene.h"

#include "math/Math.h"
#include "renderer/ObjLoader.h"

#include <filesystem>
#include <stdexcept>

namespace sr {

namespace {

std::vector<Vertex> makeSphereMesh(float radius, int latitudeSegments, int longitudeSegments);

Texture loadTextureOrCheckerboard(const wchar_t* fileName, int checkerCells)
{
    const std::filesystem::path file(fileName);
    const std::filesystem::path candidates[] = {
        std::filesystem::path(L"res") / L"Texture" / file,
        std::filesystem::path(L"..") / L"res" / L"Texture" / file,
        std::filesystem::path(L"..") / L".." / L"res" / L"Texture" / file,
        std::filesystem::path(L"..") / L".." / L".." / L"res" / L"Texture" / file,
    };

    for (const std::filesystem::path& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            try {
                return Texture::loadFromFile(candidate.c_str());
            } catch (const std::runtime_error&) {
            }
        }
    }

    return Texture::makeCheckerboard(128, 128, checkerCells);
}

std::filesystem::path findFirstObjFile()
{
    const std::filesystem::path candidates[] = {
        std::filesystem::path(L"res") / L"Model",
        std::filesystem::path(L"res") / L"Models",
        std::filesystem::path(L"..") / L"res" / L"Model",
        std::filesystem::path(L"..") / L"res" / L"Models",
        std::filesystem::path(L"..") / L".." / L"res" / L"Model",
        std::filesystem::path(L"..") / L".." / L"res" / L"Models",
        std::filesystem::path(L"..") / L".." / L".." / L"res" / L"Model",
        std::filesystem::path(L"..") / L".." / L".." / L"res" / L"Models",
    };

    for (const std::filesystem::path& directory : candidates) {
        if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
            continue;
        }

        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory)) {
            const std::filesystem::path extension = entry.path().extension();
            if (entry.is_regular_file() && (extension == L".obj" || extension == L".OBJ")) {
                return entry.path();
            }
        }
    }

    return {};
}

std::vector<Vertex> loadObjOrSphere(bool& usingObjModel)
{
    const std::filesystem::path objPath = findFirstObjFile();
    if (!objPath.empty()) {
        try {
            usingObjModel = true;
            return ObjLoader::load(objPath, { true, 0.95f, { 255, 255, 255, 255 } });
        } catch (const std::runtime_error&) {
            usingObjModel = false;
        }
    }

    usingObjModel = false;
    return makeSphereMesh(0.72f, 24, 48);
}

Vertex makeVertex(Vec3 position, Vec2 uv, Vec3 normal)
{
    return { position, uv, normalize(normal), { 255, 255, 255, 255 } };
}

std::vector<Vertex> makeSphereMesh(float radius, int latitudeSegments, int longitudeSegments)
{
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<std::size_t>(latitudeSegments) * static_cast<std::size_t>(longitudeSegments) * 6);

    const auto point = [radius](float u, float v) {
        const float phi = u * 2.0f * pi;
        const float theta = v * pi;
        const float sinTheta = std::sin(theta);
        const Vec3 normal {
            sinTheta * std::cos(phi),
            std::cos(theta),
            sinTheta * std::sin(phi),
        };
        return makeVertex(normal * radius, { u, 1.0f - v }, normal);
    };

    for (int y = 0; y < latitudeSegments; ++y) {
        const float v0 = static_cast<float>(y) / static_cast<float>(latitudeSegments);
        const float v1 = static_cast<float>(y + 1) / static_cast<float>(latitudeSegments);

        for (int x = 0; x < longitudeSegments; ++x) {
            const float u0 = static_cast<float>(x) / static_cast<float>(longitudeSegments);
            const float u1 = static_cast<float>(x + 1) / static_cast<float>(longitudeSegments);

            const Vertex p00 = point(u0, v0);
            const Vertex p10 = point(u1, v0);
            const Vertex p01 = point(u0, v1);
            const Vertex p11 = point(u1, v1);

            vertices.push_back(p00);
            vertices.push_back(p01);
            vertices.push_back(p11);

            vertices.push_back(p00);
            vertices.push_back(p11);
            vertices.push_back(p10);
        }
    }

    return vertices;
}

void addQuad(std::vector<Vertex>& vertices, Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal)
{
    vertices.push_back(makeVertex(a, { 0.0f, 0.0f }, normal));
    vertices.push_back(makeVertex(b, { 1.0f, 0.0f }, normal));
    vertices.push_back(makeVertex(c, { 1.0f, 1.0f }, normal));

    vertices.push_back(makeVertex(a, { 0.0f, 0.0f }, normal));
    vertices.push_back(makeVertex(c, { 1.0f, 1.0f }, normal));
    vertices.push_back(makeVertex(d, { 0.0f, 1.0f }, normal));
}

std::vector<Vertex> makeCubeMesh(float size)
{
    const float h = size * 0.5f;
    std::vector<Vertex> vertices;
    vertices.reserve(36);

    addQuad(vertices, { -h, -h, h }, { h, -h, h }, { h, h, h }, { -h, h, h }, { 0.0f, 0.0f, 1.0f });
    addQuad(vertices, { h, -h, -h }, { -h, -h, -h }, { -h, h, -h }, { h, h, -h }, { 0.0f, 0.0f, -1.0f });
    addQuad(vertices, { -h, -h, -h }, { -h, -h, h }, { -h, h, h }, { -h, h, -h }, { -1.0f, 0.0f, 0.0f });
    addQuad(vertices, { h, -h, h }, { h, -h, -h }, { h, h, -h }, { h, h, h }, { 1.0f, 0.0f, 0.0f });
    addQuad(vertices, { -h, h, h }, { h, h, h }, { h, h, -h }, { -h, h, -h }, { 0.0f, 1.0f, 0.0f });
    addQuad(vertices, { -h, -h, -h }, { h, -h, -h }, { h, -h, h }, { -h, -h, h }, { 0.0f, -1.0f, 0.0f });

    return vertices;
}

} // namespace

TestScene::TestScene()
    : modelTexture_(loadTextureOrCheckerboard(L"Frosted Metal Texture.jpeg", 10))
    , cubeTexture_(loadTextureOrCheckerboard(L"Brushed metal texture.jpeg", 8))
    , modelMesh_(loadObjOrSphere(usingObjModel_))
    , cubeMesh_(makeCubeMesh(1.35f))
{
    commands_[0].mesh = Mesh { modelMesh_.data(), static_cast<int>(modelMesh_.size()) };
    commands_[0].texture = &modelTexture_;
    commands_[1].mesh = Mesh { cubeMesh_.data(), static_cast<int>(cubeMesh_.size()) };
    commands_[1].texture = &cubeTexture_;
}

void TestScene::update(float deltaSeconds)
{
    rotation_ += deltaSeconds;
    commands_[0].transform = Mat4::translation({ -0.52f, 0.02f, -2.25f })
        * Mat4::rotationY(rotation_ * (usingObjModel_ ? 0.45f : 0.65f))
        * Mat4::rotationX(usingObjModel_ ? radians(-90.0f) : std::sin(rotation_ * 0.7f) * 0.18f);
    commands_[1].transform = Mat4::translation({ 0.58f, -0.02f, -3.75f })
        * Mat4::rotationY(-rotation_ * 0.45f)
        * Mat4::rotationX(0.45f)
        * Mat4::rotationZ(0.18f);
}

} // namespace sr
