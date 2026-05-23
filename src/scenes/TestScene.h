#pragma once

#include "renderer/Vertex.h"
#include "renderer/Texture.h"

#include <array>
#include <span>

namespace sr {

class TestScene {
public:
    TestScene();

    void update(float deltaSeconds);
    Mat4 viewProjection(int width, int height) const;
    std::span<const DrawCommand> drawCommands() const { return commands_; }

private:
    float rotation_ = 0.0f;
    Texture checkerboard_;
    std::array<Vertex, 3> frontTriangle_;
    std::array<Vertex, 3> backTriangle_;
    std::array<DrawCommand, 2> commands_;
};

} // namespace sr
