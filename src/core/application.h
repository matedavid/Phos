#pragma once

#include "core.h"

#include "core/layer.h"

int main(int argc, const char* argv[]);

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

  private:
    std::shared_ptr<Window> m_window = nullptr;
    std::vector<std::unique_ptr<Layer>> m_layers;
};

} // namespace Phos
