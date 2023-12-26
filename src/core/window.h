#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <functional>
#include <string_view>

#include "input/events.h"

// Forward declarations
struct GLFWwindow;

namespace Phos {

class Window {
  public:
    Window(std::string_view title, uint32_t width, uint32_t height);
    ~Window();

    void update() const;
    [[nodiscard]] bool should_close() const;
    [[nodiscard]] double get_current_time() const;

    void add_event_callback_func(std::function<void(Event&)> func);

    [[nodiscard]] static std::vector<const char*> get_vulkan_instance_extensions();
    VkResult create_surface(const VkInstance& instance, VkSurfaceKHR& surface) const;

    [[nodiscard]] uint32_t get_width() const { return m_data.width; }
    [[nodiscard]] uint32_t get_height() const { return m_data.height; }

    [[nodiscard]] GLFWwindow* handle() const { return m_window; }

  private:
    GLFWwindow* m_window;

    struct WindowData {
        uint32_t width;
        uint32_t height;

        std::function<void(Event&)> event_callback;
    };

    WindowData m_data{};
    std::vector<std::function<void(Event&)>> m_event_callback_funcs;

    void on_event(Event& event);
};

} // namespace Phos
