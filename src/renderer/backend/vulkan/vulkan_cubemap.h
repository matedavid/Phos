#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "renderer/backend/cubemap.h"

namespace Phos {

// Forward declarations
class VulkanImage;

class VulkanCubemap : public Cubemap {
  public:
    explicit VulkanCubemap(const Faces& faces);
    explicit VulkanCubemap(const Faces& faces, const std::string& directory);
    explicit VulkanCubemap(const std::string& equirectangular_path);
    ~VulkanCubemap() override;

    [[nodiscard]] std::shared_ptr<Image> get_image() const override;
    [[nodiscard]] VkImageView view() const;
    [[nodiscard]] VkSampler sampler() const;

  private:
    std::shared_ptr<VulkanImage> m_image;
    VkSampler m_sampler{VK_NULL_HANDLE};

    void init(const Faces& faces);
    static void load_face(const std::string& path,
                          std::vector<unsigned char>& data,
                          int32_t& width,
                          int32_t& height,
                          uint32_t idx);
};

} // namespace Phos
