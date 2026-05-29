#include "core/Camera.h"

#include <algorithm>
#include <cmath>

namespace sr {

void Camera::update(const InputState& input, float deltaSeconds)
{
    const float lookStep = lookSpeed_ * deltaSeconds;
    if (input.lookLeft) {
        yaw_ -= lookStep;
    }
    if (input.lookRight) {
        yaw_ += lookStep;
    }
    if (input.lookUp) {
        pitch_ += lookStep;
    }
    if (input.lookDown) {
        pitch_ -= lookStep;
    }
    if (input.mouseLook) {
        yaw_ += input.mouseDeltaX * mouseSensitivity_;
        pitch_ -= input.mouseDeltaY * mouseSensitivity_;
    }

    pitch_ = std::clamp(pitch_, radians(-86.0f), radians(86.0f));

    Vec3 movement {};
    const Vec3 f = forward();
    const Vec3 r = right();
    const Vec3 up { 0.0f, 1.0f, 0.0f };

    if (input.moveForward) {
        movement = movement + f;
    }
    if (input.moveBackward) {
        movement = movement - f;
    }
    if (input.moveRight) {
        movement = movement + r;
    }
    if (input.moveLeft) {
        movement = movement - r;
    }
    if (input.moveUp) {
        movement = movement + up;
    }
    if (input.moveDown) {
        movement = movement - up;
    }

    movement = normalize(movement);
    const float speed = moveSpeed_ * (input.speedBoost ? 3.0f : 1.0f);
    position_ = position_ + movement * speed * deltaSeconds;
}

Mat4 Camera::viewMatrix() const
{
    const Vec3 f = forward();
    const Vec3 r = right();
    const Vec3 u = normalize(cross(r, f));

    Mat4 view = Mat4::identity();
    view.m[0][0] = r.x;
    view.m[0][1] = r.y;
    view.m[0][2] = r.z;
    view.m[0][3] = -dot(r, position_);

    view.m[1][0] = u.x;
    view.m[1][1] = u.y;
    view.m[1][2] = u.z;
    view.m[1][3] = -dot(u, position_);

    view.m[2][0] = -f.x;
    view.m[2][1] = -f.y;
    view.m[2][2] = -f.z;
    view.m[2][3] = dot(f, position_);

    return view;
}

Mat4 Camera::projectionMatrix(int width, int height) const
{
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    return Mat4::perspective(fovYRadians_, aspect, nearPlane_, farPlane_);
}

Mat4 Camera::viewProjectionMatrix(int width, int height) const
{
    return projectionMatrix(width, height) * viewMatrix();
}

Vec3 Camera::forward() const
{
    const float cosPitch = std::cos(pitch_);
    return normalize({
        std::sin(yaw_) * cosPitch,
        std::sin(pitch_),
        -std::cos(yaw_) * cosPitch,
    });
}

Vec3 Camera::right() const
{
    return normalize(cross(forward(), { 0.0f, 1.0f, 0.0f }));
}

} // namespace sr
