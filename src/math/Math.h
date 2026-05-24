#pragma once

#include <cmath>

namespace sr {

constexpr float pi = 3.14159265358979323846f;

inline float radians(float degrees)
{
    return degrees * pi / 180.0f;
}

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3 operator+(Vec3 rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    Vec3 operator-(Vec3 rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
    Vec3 operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }
    Vec3 operator/(float scalar) const { return { x / scalar, y / scalar, z / scalar }; }
};

struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

inline float dot(Vec3 lhs, Vec3 rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline Vec3 cross(Vec3 lhs, Vec3 rhs)
{
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

inline Vec3 normalize(Vec3 value)
{
    const float length = std::sqrt(dot(value, value));
    if (length <= 0.000001f) {
        return {};
    }

    return value * (1.0f / length);
}

struct Mat4 {
    float m[4][4] = {};

    static Mat4 identity()
    {
        Mat4 result;
        result.m[0][0] = 1.0f;
        result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f;
        result.m[3][3] = 1.0f;
        return result;
    }

    static Mat4 translation(Vec3 value)
    {
        Mat4 result = identity();
        result.m[0][3] = value.x;
        result.m[1][3] = value.y;
        result.m[2][3] = value.z;
        return result;
    }

    static Mat4 rotationZ(float angleRadians)
    {
        Mat4 result = identity();
        const float c = std::cos(angleRadians);
        const float s = std::sin(angleRadians);
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }

    static Mat4 rotationX(float angleRadians)
    {
        Mat4 result = identity();
        const float c = std::cos(angleRadians);
        const float s = std::sin(angleRadians);
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }

    static Mat4 rotationY(float angleRadians)
    {
        Mat4 result = identity();
        const float c = std::cos(angleRadians);
        const float s = std::sin(angleRadians);
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }

    static Mat4 perspective(float fovYRadians, float aspect, float nearPlane, float farPlane)
    {
        Mat4 result;
        const float f = 1.0f / std::tan(fovYRadians * 0.5f);
        result.m[0][0] = f / aspect;
        result.m[1][1] = f;
        result.m[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
        result.m[2][3] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
        result.m[3][2] = -1.0f;
        return result;
    }

    static Mat4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        Mat4 result = identity();
        result.m[0][0] = 2.0f / (right - left);
        result.m[1][1] = 2.0f / (top - bottom);
        result.m[2][2] = -2.0f / (farPlane - nearPlane);
        result.m[0][3] = -(right + left) / (right - left);
        result.m[1][3] = -(top + bottom) / (top - bottom);
        result.m[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        return result;
    }

    static Mat4 lookAt(Vec3 eye, Vec3 target, Vec3 up)
    {
        const Vec3 f = normalize(target - eye);
        const Vec3 r = normalize(cross(f, up));
        const Vec3 u = cross(r, f);

        Mat4 result = identity();
        result.m[0][0] = r.x;
        result.m[0][1] = r.y;
        result.m[0][2] = r.z;
        result.m[0][3] = -dot(r, eye);
        result.m[1][0] = u.x;
        result.m[1][1] = u.y;
        result.m[1][2] = u.z;
        result.m[1][3] = -dot(u, eye);
        result.m[2][0] = -f.x;
        result.m[2][1] = -f.y;
        result.m[2][2] = -f.z;
        result.m[2][3] = dot(f, eye);
        return result;
    }
};

inline Mat4 operator*(const Mat4& lhs, const Mat4& rhs)
{
    Mat4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            for (int k = 0; k < 4; ++k) {
                result.m[row][col] += lhs.m[row][k] * rhs.m[k][col];
            }
        }
    }
    return result;
}

inline Vec4 operator*(const Mat4& lhs, Vec4 rhs)
{
    return {
        lhs.m[0][0] * rhs.x + lhs.m[0][1] * rhs.y + lhs.m[0][2] * rhs.z + lhs.m[0][3] * rhs.w,
        lhs.m[1][0] * rhs.x + lhs.m[1][1] * rhs.y + lhs.m[1][2] * rhs.z + lhs.m[1][3] * rhs.w,
        lhs.m[2][0] * rhs.x + lhs.m[2][1] * rhs.y + lhs.m[2][2] * rhs.z + lhs.m[2][3] * rhs.w,
        lhs.m[3][0] * rhs.x + lhs.m[3][1] * rhs.y + lhs.m[3][2] * rhs.z + lhs.m[3][3] * rhs.w,
    };
}

} // namespace sr
