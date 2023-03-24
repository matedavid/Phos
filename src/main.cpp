#include <vector>
#include <iostream>

#include <memory>

#include "core/window.h"
#include "renderer/vulkan_context.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

int main() {
    auto window = std::make_shared<Window>(WIDTH, HEIGHT);
    const auto& extensions = window->get_vulkan_instance_extensions();

    // [[maybe_unused]] VulkanContext context(extensions, window);

    while (!window->should_close()) {
        window->update();
    }

    return 0;
}