#pragma once

namespace sr {

struct InputState {
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    bool lookLeft = false;
    bool lookRight = false;
    bool lookUp = false;
    bool lookDown = false;
    bool speedBoost = false;
};

} // namespace sr
