#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

#include "renderer/vulkan_context.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

int main() {
    CORE_ASSERT(glfwInit(), "Failed to initialize GLFW");
    if (!glfwVulkanSupported()) {
        glfwTerminate();
        CORE_FAIL("GLFW does not support vulkan");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan renderer", nullptr, nullptr);

    uint32_t number_extensions = 0;
    const auto& glfw_extensions = glfwGetRequiredInstanceExtensions(&number_extensions);

    std::vector<const char*> extensions{};
    for (uint32_t i = 0; i < number_extensions; ++i) {
        extensions.push_back(glfw_extensions[i]);
    }

    [[maybe_unused]] VulkanContext context(extensions, window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}