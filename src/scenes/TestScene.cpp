#include "scenes/TestScene.h"

#include "math/Math.h"

namespace sr {

TestScene::TestScene()
    : frontTriangle_ {
        Vertex { { 0.0f, 0.72f, 0.0f }, { 245, 96, 78, 255 } },
        Vertex { { -0.78f, -0.58f, 0.0f }, { 73, 190, 255, 255 } },
        Vertex { { 0.78f, -0.58f, 0.0f }, { 250, 218, 90, 255 } },
    }
    , backTriangle_ {
        Vertex { { 0.0f, -0.72f, 0.0f }, { 76, 235, 164, 255 } },
        Vertex { { -0.92f, 0.52f, 0.0f }, { 134, 112, 255, 255 } },
        Vertex { { 0.92f, 0.52f, 0.0f }, { 255, 151, 79, 255 } },
    }
{
    commands_[0].mesh = Mesh { frontTriangle_.data(), static_cast<int>(frontTriangle_.size()) };
    commands_[1].mesh = Mesh { backTriangle_.data(), static_cast<int>(backTriangle_.size()) };
}

void TestScene::update(float deltaSeconds)
{
    rotation_ += deltaSeconds;
    commands_[0].transform = Mat4::translation({ -0.16f, 0.0f, -2.0f }) * Mat4::rotationZ(rotation_);
    commands_[1].transform = Mat4::translation({ 0.16f, 0.0f, -2.8f }) * Mat4::rotationZ(-rotation_ * 0.7f);
}

Mat4 TestScene::viewProjection(int width, int height) const
{
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    return Mat4::perspective(radians(65.0f), aspect, 0.1f, 100.0f);
}

} // namespace sr
