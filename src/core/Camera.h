#pragma once

#include "math/Math.h"
#include "platform/InputState.h"

namespace sr {

class Camera {
public:
    void update(const InputState& input, float deltaSeconds);

    Mat4 viewMatrix() const;
    Mat4 projectionMatrix(int width, int height) const;
    Mat4 viewProjectionMatrix(int width, int height) const;

private:
    Vec3 forward() const;
    Vec3 right() const;

    Vec3 position_ = { 0.0f, 0.75f, 0.0f };
    float yaw_ = 0.0f;
    float pitch_ = radians(-16.0f);
    float moveSpeed_ = 2.4f;
    float lookSpeed_ = 1.65f;
    float mouseSensitivity_ = 0.0024f;
    float fovYRadians_ = radians(65.0f);
    float nearPlane_ = 0.1f;
    float farPlane_ = 100.0f;
};

} // namespace sr
