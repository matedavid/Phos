#pragma once

#include "core.h"

#include <vector>
#include <vulkan/vulkan.h>

// Forward declarations
struct GLFWwindow;

namespace Phos {

class Window {
  public:
    Window(std::string_view title, uint32_t width, uint32_t height);
    ~Window();

    void update() const;
    [[nodiscard]] bool should_close() const;

    [[nodiscard]] std::vector<const char*> get_vulkan_instance_extensions() const;
    VkResult create_surface(const VkInstance& instance, VkSurfaceKHR& surface) const;

    [[nodiscard]] uint32_t get_width() const { return m_data.width; }
    [[nodiscard]] uint32_t get_height() const { return m_data.height; }

  private:
    GLFWwindow* m_window;

    struct WindowData {
        std::string_view title;
        uint32_t width;
        uint32_t height;
    };

    WindowData m_data{};
};

} // namespace Phos
