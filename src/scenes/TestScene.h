#pragma once

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
    Texture sphereTexture_;
    Texture cubeTexture_;
    std::vector<Vertex> sphereMesh_;
    std::vector<Vertex> cubeMesh_;
    std::array<DrawCommand, 2> commands_;
};

} // namespace sr
