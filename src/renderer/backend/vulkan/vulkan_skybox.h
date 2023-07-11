#pragma once

#include "renderer/backend/skybox.h"

#include <vulkan/vulkan.h>

namespace Phos {

// Forward declarations
class VulkanImage;

class VulkanSkybox : public Skybox {
  public:
    explicit VulkanSkybox(const Faces& faces);
    explicit VulkanSkybox(const Faces& faces, const std::string& directory);
    ~VulkanSkybox() override;

    [[nodiscard]] VkImageView view() const;
    [[nodiscard]] VkSampler sampler() const;

  private:
    std::unique_ptr<VulkanImage> m_image;
    VkSampler m_sampler{VK_NULL_HANDLE};

    void init(const Faces& faces);
    void load_face(const std::string& path, std::vector<char>& data, int32_t& width, int32_t& height);
};

} // namespace Phos
