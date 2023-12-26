#include "texture_manager.h"

#include <filesystem>

#include "renderer/backend/texture.h"

namespace Phos {

TextureManager::TextureManager() {
    m_white_texture = Texture::white(1, 1);
}

std::shared_ptr<Texture> TextureManager::acquire(const std::string& path) {
    const auto hash = hash_from_string(path);

    if (m_textures.contains(hash)) {
        return m_textures[hash];
    }

    auto texture = Texture::create(path);
    m_textures.insert(std::make_pair(hash, texture));
    return texture;
}

std::shared_ptr<Texture> TextureManager::get_white_texture() const {
    return m_white_texture;
}

std::size_t TextureManager::hash_from_string(const std::string& str) {
    constexpr auto hasher = std::hash<std::string>();
    return hasher(str);
}

} // namespace Phos
