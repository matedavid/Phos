#include "application.h"

#include "core/window.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

Application::Application(std::string_view title, uint32_t width, uint32_t height) {
    m_window = std::make_shared<Window>(title, width, height);
    VulkanContext::init(m_window);
}

Application::~Application() {
    m_layers.clear();

    VulkanContext::free();
}

void Application::run() {
    double last_time = m_window->get_current_time();

    while (!m_window->should_close()) {
        const double current_time = m_window->get_current_time();
        const double ts = current_time - last_time;
        last_time = current_time;

        for (auto& layer : m_layers) {
            layer->on_update(ts);
        }

        m_window->update();
    }
}

} // namespace Phos
