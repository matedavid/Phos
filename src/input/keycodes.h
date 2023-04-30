#pragma once

#include "core.h"

namespace Phos {

enum class Key {
    Space = 32,

    // Keyboard
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,

    // Arrows
    RightArrow = 262,
    LeftArrow = 263,
    DownArrow = 264,
    UpArrow = 265,

    // Function
    Enter = 257,
    LeftShift = 340,
    LeftControl = 341,
    RightShift = 344,
    RightControl = 345,
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 3,
};

// TODO:
// enum class HG_API ModifierKey {
//    Shift = 0x01,
//    Control = 0x02,
//    Alt = 0x04,
//    Super = 0x08,
//    CapsLock = 0x10,
//    NumLock = 0x20
//};

} // namespace Phos