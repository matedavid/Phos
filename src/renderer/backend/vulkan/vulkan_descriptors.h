#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Phos {

// Forward declarations
class VulkanDevice;

class VulkanDescriptorAllocator {
  public:
    struct PoolSizes {
        std::vector<std::pair<VkDescriptorType, float>> sizes = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f},
        };
    };

    VulkanDescriptorAllocator() = default;
    ~VulkanDescriptorAllocator();

    [[nodiscard]] bool allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set);
    void reset_pools();

  private:
    VkDescriptorPool m_current_pool{VK_NULL_HANDLE};

    PoolSizes m_descriptor_sizes{};

    std::vector<VkDescriptorPool> m_used_pools;
    std::vector<VkDescriptorPool> m_free_pools;

    VkDescriptorPool grab_pool();
};

class VulkanDescriptorLayoutCache {
  public:
    VulkanDescriptorLayoutCache() = default;
    ~VulkanDescriptorLayoutCache();

    [[nodiscard]] VkDescriptorSetLayout create_descriptor_layout(const VkDescriptorSetLayoutCreateInfo& info);

  private:
    struct DescriptorLayoutInfo {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        [[nodiscard]] bool operator==(const DescriptorLayoutInfo& other) const;
        [[nodiscard]] size_t hash() const;
    };

    struct DescriptorLayoutHash {
        size_t operator()(const DescriptorLayoutInfo& info) const { return info.hash(); }
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_layout_cache;
};

class VulkanDescriptorBuilder {
  public:
    static VulkanDescriptorBuilder begin(std::shared_ptr<VulkanDescriptorLayoutCache> cache,
                                         std::shared_ptr<VulkanDescriptorAllocator> allocator);

    [[nodiscard]] VulkanDescriptorBuilder& bind_buffer(uint32_t binding,
                                                       const VkDescriptorBufferInfo& buffer_info,
                                                       VkDescriptorType type,
                                                       VkShaderStageFlags stage_flags);

    [[nodiscard]] VulkanDescriptorBuilder& bind_image(uint32_t binding,
                                                      const VkDescriptorImageInfo& image_info,
                                                      VkDescriptorType type,
                                                      VkShaderStageFlags stage_flags);

    [[nodiscard]] bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
    [[nodiscard]] bool build(VkDescriptorSet& set);

  private:
    std::shared_ptr<VulkanDescriptorLayoutCache> m_cache;
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

} // namespace Phos
