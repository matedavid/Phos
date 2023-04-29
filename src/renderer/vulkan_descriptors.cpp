#include "vulkan_descriptors.h"

#include <algorithm>
#include <ranges>

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_context.h"

namespace Phos {

//
// DescriptorAllocator
//

static VkDescriptorPool create_pool(VkDevice device,
                                    const VulkanDescriptorAllocator::PoolSizes& poolSizes,
                                    uint32_t count,
                                    VkDescriptorPoolCreateFlags flags) {
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(poolSizes.sizes.size());

    for (const auto& sz : poolSizes.sizes)
        sizes.push_back({sz.first, uint32_t(sz.second * (float)count)});

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = flags;
    pool_info.maxSets = static_cast<uint32_t>(count);
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

    return descriptorPool;
}

VulkanDescriptorAllocator::~VulkanDescriptorAllocator() {
    for (const auto& p : m_free_pools) {
        vkDestroyDescriptorPool(VulkanContext::device->handle(), p, nullptr);
    }

    for (const auto& p : m_used_pools) {
        vkDestroyDescriptorPool(VulkanContext::device->handle(), p, nullptr);
    }
}

bool VulkanDescriptorAllocator::allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set) {
    if (m_current_pool == VK_NULL_HANDLE) {
        m_current_pool = grab_pool();
        m_used_pools.push_back(m_current_pool);
    }

    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.pSetLayouts = &layout;
    info.descriptorPool = m_current_pool;
    info.descriptorSetCount = 1;

    auto result = vkAllocateDescriptorSets(VulkanContext::device->handle(), &info, &set);
    bool needReallocate = false;

    switch (result) {
    case VK_SUCCESS:
        // all good, return
        return true;
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        // reallocate pool
        needReallocate = true;
        break;
    default:
        // unrecoverable error
        return false;
    }

    if (needReallocate) {
        // allocate a new pool and retry
        m_current_pool = grab_pool();
        m_used_pools.push_back(m_current_pool);

        result = vkAllocateDescriptorSets(VulkanContext::device->handle(), &info, &set);

        // if it still fails then we have big issues
        if (result == VK_SUCCESS)
            return true;
    }

    return false;
}

void VulkanDescriptorAllocator::reset_pools() {
    for (const auto& p : m_used_pools)
        vkResetDescriptorPool(VulkanContext::device->handle(), p, 0);

    m_free_pools = m_used_pools;
    m_used_pools.clear();
    m_current_pool = VK_NULL_HANDLE;
}

VkDescriptorPool VulkanDescriptorAllocator::grab_pool() {
    if (m_free_pools.empty())
        return create_pool(VulkanContext::device->handle(), m_descriptor_sizes, 1000, 0);

    const auto pool = m_free_pools.back();
    m_free_pools.pop_back();

    return pool;
}

//
// DescriptorLayoutCache
//

VulkanDescriptorLayoutCache::~VulkanDescriptorLayoutCache() {
    for (const auto& [_, layout] : m_layout_cache) {
        vkDestroyDescriptorSetLayout(VulkanContext::device->handle(), layout, nullptr);
    }
}

VkDescriptorSetLayout VulkanDescriptorLayoutCache::create_descriptor_layout(
    const VkDescriptorSetLayoutCreateInfo& info) {
    DescriptorLayoutInfo layout_info{};
    layout_info.bindings.reserve(info.bindingCount);

    bool is_sorted = true;
    int32_t last_binding = -1;

    for (uint32_t i = 0; i < info.bindingCount; i++) {
        layout_info.bindings.push_back(info.pBindings[i]);

        // check that the bindings are in strict increasing order
        if ((int32_t)info.pBindings[i].binding > last_binding) {
            last_binding = static_cast<int32_t>(info.pBindings[i].binding);
        } else {
            is_sorted = false;
        }
    }

    if (!is_sorted) {
        std::ranges::sort(layout_info.bindings, [](const auto& a, const auto& b) { return a.binding < b.binding; });
    }

    if (m_layout_cache.contains(layout_info)) {
        return m_layout_cache.find(layout_info)->second;
    }

    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(VulkanContext::device->handle(), &info, nullptr, &layout);

    m_layout_cache[layout_info] = layout;
    return layout;
}

bool VulkanDescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const {
    if (other.bindings.size() != bindings.size()) {
        return false;
    }

    // compare each of the bindings is the same. Bindings are sorted so they will match
    for (uint32_t i = 0; i < bindings.size(); i++) {
        if (other.bindings[i].binding != bindings[i].binding)
            return false;

        if (other.bindings[i].descriptorType != bindings[i].descriptorType)
            return false;

        if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
            return false;

        if (other.bindings[i].stageFlags != bindings[i].stageFlags)
            return false;
    }

    return true;
}

size_t VulkanDescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
    size_t result = std::hash<size_t>()(bindings.size());

    for (const VkDescriptorSetLayoutBinding& b : bindings) {
        // pack the binding data into a single int64. Not fully correct but its ok
        size_t binding_hash =
            b.binding | static_cast<uint32_t>(b.descriptorType << 8) | b.descriptorCount << 16 | b.stageFlags << 24;

        // shuffle the packed binding data and xor it with the main hash
        result ^= std::hash<size_t>()(binding_hash);
    }

    return result;
}

//
// Descriptor Builder
//

VulkanDescriptorBuilder VulkanDescriptorBuilder::begin(std::shared_ptr<VulkanDescriptorLayoutCache> cache,
                                                       std::shared_ptr<VulkanDescriptorAllocator> allocator) {
    VulkanDescriptorBuilder builder{};

    builder.m_cache = std::move(cache);
    builder.m_allocator = std::move(allocator);

    return builder;
}

VulkanDescriptorBuilder& VulkanDescriptorBuilder::bind_buffer(uint32_t binding,
                                                              const VkDescriptorBufferInfo& buffer_info,
                                                              VkDescriptorType type,
                                                              VkShaderStageFlags stage_flags) {
    // create the descriptor binding for the layout
    VkDescriptorSetLayoutBinding new_binding{};
    new_binding.descriptorCount = 1;
    new_binding.descriptorType = type;
    new_binding.pImmutableSamplers = nullptr;
    new_binding.stageFlags = stage_flags;
    new_binding.binding = binding;

    bindings.push_back(new_binding);

    // create the descriptor write
    VkWriteDescriptorSet new_write{};
    new_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    new_write.pNext = nullptr;
    new_write.descriptorCount = 1;
    new_write.descriptorType = type;
    new_write.pBufferInfo = &buffer_info;
    new_write.dstBinding = binding;

    writes.push_back(new_write);

    return *this;
}

VulkanDescriptorBuilder& VulkanDescriptorBuilder::bind_image(uint32_t binding,
                                                             const VkDescriptorImageInfo& image_info,
                                                             VkDescriptorType type,
                                                             VkShaderStageFlags stage_flags) {
    VkDescriptorSetLayoutBinding new_binding{};
    new_binding.descriptorCount = 1;
    new_binding.descriptorType = type;
    new_binding.pImmutableSamplers = nullptr;
    new_binding.stageFlags = stage_flags;
    new_binding.binding = binding;

    bindings.push_back(new_binding);

    VkWriteDescriptorSet new_write{};
    new_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    new_write.pNext = nullptr;
    new_write.descriptorCount = 1;
    new_write.descriptorType = type;
    new_write.pImageInfo = &image_info;
    new_write.dstBinding = binding;

    writes.push_back(new_write);

    return *this;
}

bool VulkanDescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout) {
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.pNext = nullptr;
    layout_info.pBindings = bindings.data();
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());

    layout = m_cache->create_descriptor_layout(layout_info);

    if (!m_allocator->allocate(layout, set)) {
        return false;
    }

    for (VkWriteDescriptorSet& w : writes) {
        w.dstSet = set;
    }

    vkUpdateDescriptorSets(VulkanContext::device->handle(), (uint32_t)writes.size(), writes.data(), 0, nullptr);

    return true;
}

bool VulkanDescriptorBuilder::build(VkDescriptorSet& set) {
    VkDescriptorSetLayout layout;
    return build(set, layout);
}

} // namespace Phos
