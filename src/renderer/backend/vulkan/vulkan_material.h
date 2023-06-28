#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include "renderer/backend/material.h"
#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

// Forward declarations
class VulkanShader;
class VulkanDescriptorAllocator;
struct VulkanDescriptorInfo;

class VulkanMaterial : public Material {
  public:
    explicit VulkanMaterial(const Definition& definition);
    ~VulkanMaterial() override = default;

    [[nodiscard]] VkDescriptorSet get_set() const { return m_set; }

  private:
    Definition m_definition;

    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorImageInfo>> m_image_descriptor_info;

    VkDescriptorSet m_set{VK_NULL_HANDLE};
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;
};

} // namespace Phos
