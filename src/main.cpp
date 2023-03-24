#include <iostream>

#include <memory>

#include "core/window.h"
#include "renderer/vulkan_context.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

int main() {
    auto window = std::make_shared<Window>(WIDTH, HEIGHT);

    [[maybe_unused]] VulkanContext context(window);

    while (!window->should_close()) {
        window->update();
    }

    return 0;
}