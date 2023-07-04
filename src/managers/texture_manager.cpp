#include "texture_manager.h"

#include <filesystem>

#include "renderer/backend/texture.h"

namespace Phos {

TextureManager::TextureManager() {
    // TODO: Create white texture
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

std::unique_ptr<TextureManager> TextureManager::m_instance = nullptr;

const std::unique_ptr<TextureManager>& TextureManager::instance() {
    if (m_instance == nullptr) {
        m_instance = std::make_unique<TextureManager>();
    }

    return m_instance;
}

std::size_t TextureManager::hash_from_string(const std::string& str) {
    auto hasher = std::hash<std::string>();
    return hasher(str);
}

} // namespace Phos
