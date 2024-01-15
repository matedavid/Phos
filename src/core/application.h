#pragma once

#include <memory>
#include <vector>

#include "core/layer.h"
#include "input/events.h"

// int main(int argc, const char* argv[]);

namespace Phos {

// Forward declarations
class Window;

class Application {
  public:
    Application(std::string_view title, uint32_t width, uint32_t height);
    ~Application();

    void run();

    template <typename T>
    void push_layer() {
        static_assert(std::is_base_of<Layer, T>());
        m_layers.push_back(std::make_unique<T>());
    }

    [[nodiscard]] const std::shared_ptr<Window>& get_window() const { return m_window; }

    [[nodiscard]] static Application* instance() { return m_instance; }

  private:
    std::shared_ptr<Window> m_window = nullptr;
    std::vector<std::unique_ptr<Layer>> m_layers;

    static Application* m_instance;

    void on_event(Event& event);
};

} // namespace Phos
