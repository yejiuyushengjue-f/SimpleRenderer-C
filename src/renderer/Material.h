#pragma once

#include "core/Color.h"

namespace sr {

class Texture;

struct Material {
    Color ambientColor = { 255, 255, 255, 255 };
    Color diffuseColor = { 255, 255, 255, 255 };
    Color specularColor = { 255, 255, 255, 255 };
    const Texture* diffuseTexture = nullptr;
    float ambientStrength = 0.24f;
    float diffuseStrength = 1.0f;
    float specularStrength = 0.48f;
    float shininess = 48.0f;
};

} // namespace sr
