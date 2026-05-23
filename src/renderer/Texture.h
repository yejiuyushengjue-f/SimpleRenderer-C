#pragma once

#include "core/Color.h"
#include "math/Math.h"

#include <vector>

namespace sr {

class Texture {
public:
    Texture() = default;
    Texture(int width, int height, std::vector<Color> pixels);

    int width() const { return width_; }
    int height() const { return height_; }
    bool empty() const { return pixels_.empty(); }

    Color sample(Vec2 uv) const;

    static Texture makeCheckerboard(int width, int height, int cells);

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Color> pixels_;
};

} // namespace sr
