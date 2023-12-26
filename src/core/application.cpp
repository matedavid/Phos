#include "application.h"

#include <ranges>

#include "utility/logging.h"
#include "core/window.h"
#include "renderer/backend/renderer.h"
#include "scripting/scripting_engine.h"

namespace Phos {

Application* Application::m_instance = nullptr;

Application::Application(std::string_view title, uint32_t width, uint32_t height) {
    PHOS_LOG_SETUP;

    m_window = std::make_shared<Window>(title, width, height);
    m_window->add_event_callback_func([&](Event& event) { on_event(event); });

    Renderer::initialize(RendererConfig{
        .graphics_api = GraphicsAPI::Vulkan,
        .window = m_window,
        .num_frames = 2,
    });

    ScriptingEngine::initialize();

    m_instance = this;
}

Application::~Application() {
    m_layers.clear();

    ScriptingEngine::shutdown();
    Renderer::shutdown();

    m_instance = nullptr;
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
        // PHOS_PROFILE_FRAMEMARK;
    }
}

void Application::on_event(Event& event) {
    for (auto& layer : std::views::reverse(m_layers)) {
        if (event.handled)
            break;

        switch (event.get_type()) {
        default:
        case EventType::WindowResize: {
            auto window_resized = dynamic_cast<WindowResizeEvent&>(event);
            layer->on_window_resized(window_resized);
            break;
        }
        case EventType::MouseMoved: {
            auto mouse_moved = dynamic_cast<MouseMovedEvent&>(event);
            layer->on_mouse_moved(mouse_moved);
            break;
        }
        case EventType::MouseButtonPressed: {
            auto mouse_pressed = dynamic_cast<MousePressedEvent&>(event);
            layer->on_mouse_pressed(mouse_pressed);
            break;
        }
        case EventType::MouseButtonReleased: {
            auto mouse_released = dynamic_cast<MouseReleasedEvent&>(event);
            layer->on_mouse_released(mouse_released);
            break;
        }
        case EventType::MouseScrolled: {
            auto mouse_scrolled = dynamic_cast<MouseScrolledEvent&>(event);
            layer->on_mouse_scrolled(mouse_scrolled);
            break;
        }
        case EventType::KeyPressed: {
            auto key_pressed = dynamic_cast<KeyPressedEvent&>(event);
            layer->on_key_pressed(key_pressed);
            break;
        }
        case EventType::KeyReleased: {
            auto key_released = dynamic_cast<KeyReleasedEvent&>(event);
            layer->on_key_released(key_released);
            break;
        }
        case EventType::KeyRepeated: {
            auto key_repeat = dynamic_cast<KeyRepeatEvent&>(event);
            layer->on_key_repeat(key_repeat);
            break;
        }
        }
    }
}

} // namespace Phos
