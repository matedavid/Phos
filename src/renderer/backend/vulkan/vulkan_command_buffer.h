#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <functional>

namespace Phos {

class VulkanCommandBuffer {
  public:
    explicit VulkanCommandBuffer(VkCommandBuffer command_buffer);
    ~VulkanCommandBuffer() = default;

    void record(const std::function<void(void)>& func) const;
    void record_single_time(const std::function<void(const VulkanCommandBuffer&)>& func) const;

    [[nodiscard]] VkCommandBuffer handle() const { return m_command_buffer; }

  private:
    VkCommandBuffer m_command_buffer;

    void begin(bool one_time = false) const;
    void end() const;
};

} // namespace Phos
