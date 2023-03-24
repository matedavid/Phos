#include "glfw_window.h"

#include <GLFW/glfw3.h>

#ifdef GLFW_WINDOW

GLFWWindow::GLFWWindow(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
    CORE_ASSERT(glfwInit(), "Failed to initialize GLFW")
    if (!glfwVulkanSupported()) {
        glfwTerminate();
        CORE_FAIL("GLFW does not support vulkan");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow((int32_t)width, (int32_t)height, "Window", nullptr, nullptr);
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

#endif
