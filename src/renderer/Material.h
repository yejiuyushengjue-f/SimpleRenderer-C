#pragma once

#include "core/Color.h"

namespace sr {

class Texture;

struct Material {
    Color ambientColor = { 255, 255, 255, 255 };
    Color diffuseColor = { 255, 255, 255, 255 };
    Color specularColor = { 255, 255, 255, 255 };
    const Texture* diffuseTexture = nullptr;
    const Texture* normalTexture = nullptr;
    float ambientStrength = 0.24f;
    float diffuseStrength = 1.0f;
    float specularStrength = 0.48f;
    float shininess = 48.0f;
    float normalStrength = 1.0f;
};

} // namespace sr
