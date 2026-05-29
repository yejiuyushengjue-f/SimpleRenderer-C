#pragma once

#include "renderer/Vertex.h"

#include <filesystem>
#include <vector>

namespace sr {

struct ObjLoadOptions {
    bool normalizeToUnit = true;
    float targetRadius = 0.8f;
    Color vertexColor = { 255, 255, 255, 255 };
};

class ObjLoader {
public:
    static std::vector<Vertex> load(const std::filesystem::path& path, ObjLoadOptions options = {});
};

} // namespace sr
