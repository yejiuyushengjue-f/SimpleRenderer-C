#include "core/Framebuffer.h"

#include <algorithm>
#include <limits>

namespace sr {

Framebuffer::Framebuffer(int width, int height)
    : width_(width)
    , height_(height)
    , pixels_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height))
    , depth_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), std::numeric_limits<float>::infinity())
{
}

void Framebuffer::clear(Color color)
{
    std::fill(pixels_.begin(), pixels_.end(), color.toBGRA());
    clearDepth(std::numeric_limits<float>::infinity());
}

void Framebuffer::clearDepth(float depth)
{
    std::fill(depth_.begin(), depth_.end(), depth);
}

void Framebuffer::setPixel(int x, int y, Color color)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }

    pixels_[indexOf(x, y)] = color.toBGRA();
}

bool Framebuffer::setPixelIfCloser(int x, int y, float depth, Color color)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return false;
    }

    const std::size_t index = indexOf(x, y);
    if (depth >= depth_[index]) {
        return false;
    }

    depth_[index] = depth;
    pixels_[index] = color.toBGRA();
    return true;
}

std::size_t Framebuffer::indexOf(int x, int y) const
{
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
}

} // namespace sr
