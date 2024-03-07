#pragma once

#include "input/keycodes.h"

namespace Phos {

class Input {
  public:
    [[nodiscard]] static bool is_key_pressed(Key key);
    [[nodiscard]] static bool is_mouse_button_pressed(MouseButton button);

    [[nodiscard]] static float horizontal_axis_change();
    [[nodiscard]] static float vertical_axis_change();
};

}; // namespace Phos
