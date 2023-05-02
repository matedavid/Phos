#include "input.h"

#include "core/application.h"
#include "core/window.h"

#include "GLFW/glfw3.h"

namespace Phos {

bool Input::is_key_pressed(Key key) {
    const auto& window = Application::instance()->get_window()->handle();

    const auto glfw_key = static_cast<int32_t>(key);
    const auto status = glfwGetKey(window, glfw_key);
    return status == GLFW_PRESS || status == GLFW_REPEAT;
}

bool Input::is_mouse_button_pressed(MouseButton button) {
    const auto& window = Application::instance()->get_window()->handle();

    const auto glfw_button = static_cast<int32_t>(button);
    const auto status = glfwGetMouseButton(window, glfw_button);
    return status == GLFW_PRESS;
}

} // namespace Phos