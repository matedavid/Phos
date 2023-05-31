#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <functional>

#include "renderer/backend/command_buffer.h"

#include "renderer/backend/vulkan/vulkan_queue.h"

namespace Phos {

class VulkanCommandBuffer : public CommandBuffer {
  public:
    explicit VulkanCommandBuffer();
    explicit VulkanCommandBuffer(VulkanQueue::Type type);
    ~VulkanCommandBuffer() override;

    void record(const std::function<void(void)>& func) const override;
    static void submit_single_time(VulkanQueue::Type type,
                                   const std::function<void(const std::shared_ptr<VulkanCommandBuffer>&)>& func);

    [[nodiscard]] VkCommandBuffer handle() const { return m_command_buffer; }

  private:
    VkCommandBuffer m_command_buffer;
    VulkanQueue::Type m_type;

    void begin(bool one_time = false) const;
    void end() const;
};

} // namespace Phos
