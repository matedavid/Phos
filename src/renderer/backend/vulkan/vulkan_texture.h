#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "renderer/backend/texture.h"

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanImage;

class VulkanTexture : public Texture {
  public:
    explicit VulkanTexture(const std::string& path);
    explicit VulkanTexture(uint32_t width, uint32_t height);
    explicit VulkanTexture(const std::vector<char>& data, uint32_t width, uint32_t height);
    explicit VulkanTexture(const std::shared_ptr<Image>& image);
    ~VulkanTexture() override;

    [[nodiscard]] std::shared_ptr<Image> get_image() const override;
    [[nodiscard]] VkSampler sampler() const { return m_sampler; }

  private:
    std::shared_ptr<VulkanImage> m_image;

    VkSampler m_sampler{};
};

} // namespace Phos
