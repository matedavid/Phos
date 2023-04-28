#include <memory>

#include "core/window.h"
#include "renderer/vulkan_renderer.h"
#include "renderer/vulkan_context.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

int main() {
    auto window = std::make_shared<Window>(WIDTH, HEIGHT);

    VulkanContext::init(window);

    auto* renderer = new VulkanRenderer(window);

    while (!window->should_close()) {
        renderer->update();

        window->update();
    }

    delete renderer;

    VulkanContext::free();

    return 0;
}