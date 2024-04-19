#pragma once

#include <imgui.h>

#include <memory>

namespace Phos {

// Forward declarations
class Window;
class Texture;

} // namespace Phos

class INativeImGuiImpl {
  public:
    virtual ~INativeImGuiImpl() = default;

    virtual void new_frame() = 0;
    virtual void render_frame(ImDrawData* draw_data) = 0;
    virtual void present_frame() = 0;

    [[nodiscard]] virtual ImTextureID add_texture(const std::shared_ptr<Phos::Texture>& texture) = 0;
    virtual void remove_texture(ImTextureID texture_id) = 0;
};

class ImGuiImpl {
  public:
    static void initialize(const std::shared_ptr<Phos::Window>& window);
    static void shutdown();

    static void new_frame();
    static void render_frame(ImDrawData* draw_data);
    static void present_frame();

    [[nodiscard]] static ImTextureID add_texture(const std::shared_ptr<Phos::Texture>& texture);
    static void remove_texture(ImTextureID texture_id);

  protected:
    static std::shared_ptr<INativeImGuiImpl> m_native_impl;
};
