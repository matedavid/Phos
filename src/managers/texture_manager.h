#pragma once

#include "core.h"

#include <unordered_map>

namespace Phos {

// Forward declarations
class Texture;

class TextureManager {
  public:
    TextureManager();
    ~TextureManager() = default;

    [[nodiscard]] std::shared_ptr<Texture> acquire(const std::string& path);
    [[nodiscard]] std::shared_ptr<Texture> get_white_texture() const;

    [[nodiscard]] static const std::unique_ptr<TextureManager>& instance();

  private:
    std::unordered_map<std::size_t, std::shared_ptr<Texture>> m_textures;
    std::shared_ptr<Texture> m_white_texture;

    [[nodiscard]] static std::size_t hash_from_string(const std::string& str);

    static std::unique_ptr<TextureManager> m_instance;
};

} // namespace Phos
