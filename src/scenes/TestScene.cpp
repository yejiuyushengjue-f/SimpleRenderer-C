#include "scenes/TestScene.h"

#include "math/Math.h"

namespace sr {

TestScene::TestScene()
    : checkerboard_(Texture::makeCheckerboard(128, 128, 8))
    , frontTriangle_ {
        Vertex { { 0.0f, 0.72f, -0.35f }, { 0.5f, 1.2f }, { 0.0f, 0.0f, 1.0f }, { 255, 255, 255, 255 } },
        Vertex { { -0.9f, -0.62f, 0.25f }, { -0.15f, -0.15f }, { -0.25f, -0.15f, 1.0f }, { 255, 180, 180, 255 } },
        Vertex { { 0.9f, -0.62f, 0.25f }, { 1.15f, -0.15f }, { 0.25f, -0.15f, 1.0f }, { 180, 220, 255, 255 } },
    }
    , backTriangle_ {
        Vertex { { 0.0f, -0.72f, 0.15f }, { 0.5f, -0.25f }, { 0.0f, 0.0f, 1.0f }, { 165, 255, 204, 255 } },
        Vertex { { -0.92f, 0.52f, -0.1f }, { -0.25f, 1.15f }, { -0.2f, 0.2f, 1.0f }, { 220, 190, 255, 255 } },
        Vertex { { 0.92f, 0.52f, -0.1f }, { 1.25f, 1.15f }, { 0.2f, 0.2f, 1.0f }, { 255, 209, 168, 255 } },
    }
{
    commands_[0].mesh = Mesh { frontTriangle_.data(), static_cast<int>(frontTriangle_.size()) };
    commands_[0].texture = &checkerboard_;
    commands_[1].mesh = Mesh { backTriangle_.data(), static_cast<int>(backTriangle_.size()) };
    commands_[1].texture = &checkerboard_;
}

void TestScene::update(float deltaSeconds)
{
    rotation_ += deltaSeconds;
    commands_[0].transform = Mat4::translation({ -0.16f, 0.0f, -2.0f })
        * Mat4::rotationY(std::sin(rotation_) * 0.55f)
        * Mat4::rotationZ(rotation_);
    commands_[1].transform = Mat4::translation({ 0.16f, 0.0f, -2.8f })
        * Mat4::rotationX(std::sin(rotation_ * 0.6f) * 0.45f)
        * Mat4::rotationZ(-rotation_ * 0.7f);
}

Mat4 TestScene::viewProjection(int width, int height) const
{
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    return Mat4::perspective(radians(65.0f), aspect, 0.1f, 100.0f);
}

} // namespace sr
