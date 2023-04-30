#pragma once

#include "core.h"

#include "input/keycodes.h"

namespace Phos {

class Input {
  public:
    [[nodiscard]] static bool is_key_pressed(Key key);
    [[nodiscard]] static bool is_mouse_button_pressed(MouseButton button);
};

}; // namespace Phos
