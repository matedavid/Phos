#include "glfw_window.h"

#ifdef GLFW_WINDOW

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

GLFWWindow::GLFWWindow(uint32_t width, uint32_t height) {
    CORE_ASSERT(glfwInit(), "Failed to initialize GLFW")
    if (!glfwVulkanSupported()) {
        glfwTerminate();
        CORE_FAIL("GLFW does not support vulkan");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow((int32_t)width, (int32_t)height, "Window", nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, &m_data);

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int32_t w, int32_t h) {
        const auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data->width = (uint32_t)w;
        data->height = (uint32_t)h;
    });

    m_data.width = width;
    m_data.height = height;
}

GLFWWindow::~GLFWWindow() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void GLFWWindow::update() const {
    glfwPollEvents();
}

bool GLFWWindow::should_close() const {
    return glfwWindowShouldClose(m_window);
}

std::vector<const char*> GLFWWindow::get_vulkan_instance_extensions() const {
    uint32_t number_extensions = 0;
    const auto& glfw_extensions = glfwGetRequiredInstanceExtensions(&number_extensions);

    std::vector<const char*> extensions{};
    for (uint32_t i = 0; i < number_extensions; ++i) {
        extensions.push_back(glfw_extensions[i]);
    }

    return extensions;
}

VkResult GLFWWindow::create_surface(const VkInstance& instance, VkSurfaceKHR& surface) const {
    return glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
}

#endif
