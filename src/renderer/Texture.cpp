#include "renderer/Texture.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace sr {

Texture::Texture(int width, int height, std::vector<Color> pixels)
    : width_(width)
    , height_(height)
    , pixels_(std::move(pixels))
{
    if (width_ <= 0 || height_ <= 0 || pixels_.size() != static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)) {
        throw std::runtime_error("Invalid texture dimensions or pixel data.");
    }
}

Color Texture::sample(Vec2 uv) const
{
    if (empty()) {
        return { 255, 255, 255, 255 };
    }

    uv.x = uv.x - std::floor(uv.x);
    uv.y = uv.y - std::floor(uv.y);

    const int x = std::clamp(static_cast<int>(uv.x * static_cast<float>(width_)), 0, width_ - 1);
    const int y = std::clamp(static_cast<int>((1.0f - uv.y) * static_cast<float>(height_)), 0, height_ - 1);
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

Texture Texture::makeCheckerboard(int width, int height, int cells)
{
    std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    const int cellWidth = std::max(1, width / cells);
    const int cellHeight = std::max(1, height / cells);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool dark = ((x / cellWidth) + (y / cellHeight)) % 2 == 0;
            const bool line = x % cellWidth == 0 || y % cellHeight == 0;

            Color color = dark ? Color { 38, 42, 58, 255 } : Color { 228, 236, 246, 255 };
            if (line) {
                color = { 255, 196, 87, 255 };
            }

            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = color;
        }
    }

    return Texture(width, height, std::move(pixels));
}

} // namespace sr
