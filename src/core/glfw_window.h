#pragma once

#include "core.h"

#include "core/window.h"

#ifdef GLFW_WINDOW

// Forward declarations
class GLFWwindow;

class GLFWWindow : public INativeWindow {
  public:
    GLFWWindow(uint32_t window, uint32_t height);
    ~GLFWWindow() override;

    void update() const override;
    [[nodiscard]] bool should_close() const override;

    [[nodiscard]] uint32_t get_width() const override { return m_width; }
    [[nodiscard]] uint32_t get_height() const override { return m_height; }

    [[nodiscard]] std::vector<const char*> get_vulkan_instance_extensions() const override;

    [[nodiscard]] GLFWwindow* handle() const override { return m_window; }

  private:
    GLFWwindow* m_window;

    uint32_t m_width;
    uint32_t m_height;
};

#endif