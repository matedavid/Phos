#include "window.h"

#include <memory>

#include "core/glfw_window.h"
#include "renderer/vulkan_instance.h"

Window::Window(uint32_t width, uint32_t height) {
#ifdef GLFW_WINDOW
    m_window = std::make_unique<GLFWWindow>(width, height);
#endif
}

void Window::update() const {
    m_window->update();
}

bool Window::should_close() const {
    return m_window->should_close();
}

std::vector<const char*> Window::get_vulkan_instance_extensions() const {
    return m_window->get_vulkan_instance_extensions();
}


VkResult Window::create_surface(const VkInstance& instance, VkSurfaceKHR& surface) const {
    return m_window->create_surface(instance, surface);
}

uint32_t Window::get_width() const {
    return m_window->get_width();
}

uint32_t Window::get_height() const {
    return m_window->get_height();
}

NATIVE_WINDOW_TYPE Window::handle() const {
    return m_window->handle();
}
