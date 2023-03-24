#pragma once

#include "core.h"

#include <vector>

#define GLFW_WINDOW

#ifdef GLFW_WINDOW
#define NATIVE_WINDOW_TYPE GLFWwindow*
#endif

// Forward declarations
class GLFWwindow;

class INativeWindow {
  public:
    virtual ~INativeWindow() {}

    virtual void update() const = 0;
    [[nodiscard]] virtual bool should_close() const = 0;

    [[nodiscard]] virtual uint32_t get_width() const = 0;
    [[nodiscard]] virtual uint32_t get_height() const = 0;

    [[nodiscard]] virtual std::vector<const char*> get_vulkan_instance_extensions() const = 0;

    [[nodiscard]] virtual NATIVE_WINDOW_TYPE handle() const = 0;
};

class Window {
  public:
    Window(uint32_t width, uint32_t height);
    ~Window() = default;

    void update() const;
    [[nodiscard]] bool should_close() const;

    [[nodiscard]] std::vector<const char*> get_vulkan_instance_extensions() const;

    [[nodiscard]] uint32_t get_width() const;
    [[nodiscard]] uint32_t get_height() const;

    [[nodiscard]] NATIVE_WINDOW_TYPE handle() const;

  private:
    std::unique_ptr<INativeWindow> m_window;
};
