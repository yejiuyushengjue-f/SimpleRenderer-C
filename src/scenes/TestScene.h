#pragma once

#include "renderer/Material.h"
#include "renderer/Texture.h"
#include "renderer/Vertex.h"

#include <array>
#include <span>
#include <vector>

namespace sr {

class TestScene {
public:
    TestScene();

    void update(float deltaSeconds);
    std::span<const DrawCommand> drawCommands() const { return commands_; }

private:
    float rotation_ = 0.0f;
    bool usingObjModel_ = false;
    Texture modelTexture_;
    Texture cubeTexture_;
    Texture groundTexture_;
    Material modelMaterial_;
    Material cubeMaterial_;
    Material groundMaterial_;
    std::vector<Vertex> modelMesh_;
    std::vector<Vertex> cubeMesh_;
    std::vector<Vertex> groundMesh_;
    std::array<DrawCommand, 3> commands_;
};

} // namespace sr
