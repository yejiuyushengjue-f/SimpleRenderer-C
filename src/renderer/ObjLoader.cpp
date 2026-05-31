#include "renderer/ObjLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sr {

namespace {

struct ObjIndex {
    int position = -1;
    int uv = -1;
    int normal = -1;
};

int resolveIndex(int index, int count)
{
    if (index > 0) {
        return index - 1;
    }
    if (index < 0) {
        return count + index;
    }
    return -1;
}

ObjIndex parseFaceToken(const std::string& token, int positionCount, int uvCount, int normalCount)
{
    ObjIndex result;
    std::string parts[3];
    int partIndex = 0;

    for (char ch : token) {
        if (ch == '/') {
            ++partIndex;
            if (partIndex >= 3) {
                break;
            }
        } else {
            parts[partIndex].push_back(ch);
        }
    }

    if (!parts[0].empty()) {
        result.position = resolveIndex(std::stoi(parts[0]), positionCount);
    }
    if (!parts[1].empty()) {
        result.uv = resolveIndex(std::stoi(parts[1]), uvCount);
    }
    if (!parts[2].empty()) {
        result.normal = resolveIndex(std::stoi(parts[2]), normalCount);
    }

    return result;
}

bool validIndex(int index, int count)
{
    return index >= 0 && index < count;
}

Vertex buildVertex(
    ObjIndex index,
    const std::vector<Vec3>& positions,
    const std::vector<Vec2>& uvs,
    const std::vector<Vec3>& normals,
    Vec3 fallbackNormal,
    Color color)
{
    if (!validIndex(index.position, static_cast<int>(positions.size()))) {
        throw std::runtime_error("OBJ face references an invalid position index.");
    }

    const Vec3 position = positions[static_cast<std::size_t>(index.position)];
    const Vec2 uv = validIndex(index.uv, static_cast<int>(uvs.size()))
        ? uvs[static_cast<std::size_t>(index.uv)]
        : Vec2 {};
    const Vec3 normal = validIndex(index.normal, static_cast<int>(normals.size()))
        ? normals[static_cast<std::size_t>(index.normal)]
        : fallbackNormal;

    return { position, uv, normalize(normal), color };
}

void normalizeMesh(std::vector<Vertex>& vertices, float targetRadius)
{
    if (vertices.empty()) {
        return;
    }

    Vec3 minBounds {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
    };
    Vec3 maxBounds {
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
    };

    for (const Vertex& vertex : vertices) {
        minBounds.x = std::min(minBounds.x, vertex.position.x);
        minBounds.y = std::min(minBounds.y, vertex.position.y);
        minBounds.z = std::min(minBounds.z, vertex.position.z);
        maxBounds.x = std::max(maxBounds.x, vertex.position.x);
        maxBounds.y = std::max(maxBounds.y, vertex.position.y);
        maxBounds.z = std::max(maxBounds.z, vertex.position.z);
    }

    const Vec3 center = (minBounds + maxBounds) * 0.5f;
    float radius = 0.0f;
    for (const Vertex& vertex : vertices) {
        const Vec3 offset = vertex.position - center;
        radius = std::max(radius, std::sqrt(dot(offset, offset)));
    }

    if (radius <= 0.000001f) {
        return;
    }

    const float scale = targetRadius / radius;
    for (Vertex& vertex : vertices) {
        vertex.position = (vertex.position - center) * scale;
    }
}

} // namespace

std::vector<Vertex> ObjLoader::load(const std::filesystem::path& path, ObjLoadOptions options)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open OBJ file.");
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> uvs;
    std::vector<Vec3> normals;
    std::vector<Vertex> vertices;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "v") {
            Vec3 position;
            stream >> position.x >> position.y >> position.z;
            positions.push_back(position);
        } else if (type == "vt") {
            Vec2 uv;
            stream >> uv.x >> uv.y;
            uvs.push_back(uv);
        } else if (type == "vn") {
            Vec3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normalize(normal));
        } else if (type == "f") {
            std::vector<ObjIndex> face;
            std::string token;
            while (stream >> token) {
                face.push_back(parseFaceToken(token, static_cast<int>(positions.size()), static_cast<int>(uvs.size()), static_cast<int>(normals.size())));
            }

            if (face.size() < 3) {
                continue;
            }

            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                const ObjIndex triangle[3] = { face[0], face[i], face[i + 1] };
                if (!validIndex(triangle[0].position, static_cast<int>(positions.size()))
                    || !validIndex(triangle[1].position, static_cast<int>(positions.size()))
                    || !validIndex(triangle[2].position, static_cast<int>(positions.size()))) {
                    continue;
                }

                const Vec3 p0 = positions[static_cast<std::size_t>(triangle[0].position)];
                const Vec3 p1 = positions[static_cast<std::size_t>(triangle[1].position)];
                const Vec3 p2 = positions[static_cast<std::size_t>(triangle[2].position)];
                const Vec3 fallbackNormal = normalize(cross(p1 - p0, p2 - p0));

                vertices.push_back(buildVertex(triangle[0], positions, uvs, normals, fallbackNormal, options.vertexColor));
                vertices.push_back(buildVertex(triangle[1], positions, uvs, normals, fallbackNormal, options.vertexColor));
                vertices.push_back(buildVertex(triangle[2], positions, uvs, normals, fallbackNormal, options.vertexColor));
            }
        }
    }

    if (vertices.empty()) {
        throw std::runtime_error("OBJ file did not contain renderable triangles.");
    }

    if (options.normalizeToUnit) {
        normalizeMesh(vertices, options.targetRadius);
    }
    assignMeshTangents(vertices.data(), static_cast<int>(vertices.size()));

    return vertices;
}

} // namespace sr
