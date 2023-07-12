#pragma once

#include "renderer/backend/cubemap.h"

#include <vulkan/vulkan.h>

namespace Phos {

// Forward declarations
class VulkanImage;

class VulkanCubemap : public Cubemap {
  public:
    explicit VulkanCubemap(const Faces& faces);
    explicit VulkanCubemap(const Faces& faces, const std::string& directory);
    ~VulkanCubemap() override;

    [[nodiscard]] VkImageView view() const;
    [[nodiscard]] VkSampler sampler() const;

  private:
    std::unique_ptr<VulkanImage> m_image;
    VkSampler m_sampler{VK_NULL_HANDLE};

    void init(const Faces& faces);
    static void load_face(const std::string& path, std::vector<char>& data, int32_t& width, int32_t& height);
};

} // namespace Phos
