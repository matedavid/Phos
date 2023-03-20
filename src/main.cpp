#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan renderer", nullptr, nullptr);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    uint32_t number_extensions;
    const auto& glfw_extensions = glfwGetRequiredInstanceExtensions(&number_extensions);

    std::vector<const char*> extensions{};
    for (uint32_t i = 0; i < number_extensions; ++i) {
        extensions.push_back(glfw_extensions[i]);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    return 0;
}