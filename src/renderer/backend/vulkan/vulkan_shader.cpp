#include "vulkan_shader.h"

#include <ranges>
#include <algorithm>
#include <fstream>
#include <spirv_reflect.h>
#include <vulkan/vk_format_utils.h>

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_utils.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

#define SPIRV_REFLECT_CHECK(expression)             \
    if (expression != SPV_REFLECT_RESULT_SUCCESS) { \
        PS_FAIL("Spirv-reflect call failed");       \
    }

VulkanShader::VulkanShader(const std::string& vertex_path, const std::string& fragment_path) {
    const auto vertex_src = read_shader_file(vertex_path);
    const auto fragment_src = read_shader_file(fragment_path);

    VkShaderModuleCreateInfo vertex_create_info{};
    vertex_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_create_info.codeSize = vertex_src.size();
    vertex_create_info.pCode = reinterpret_cast<const uint32_t*>(vertex_src.data());

    VkShaderModuleCreateInfo fragment_create_info{};
    fragment_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragment_create_info.codeSize = fragment_src.size();
    fragment_create_info.pCode = reinterpret_cast<const uint32_t*>(fragment_src.data());

    VK_CHECK(vkCreateShaderModule(VulkanContext::device->handle(), &vertex_create_info, nullptr, &m_vertex_shader))
    VK_CHECK(vkCreateShaderModule(VulkanContext::device->handle(), &fragment_create_info, nullptr, &m_fragment_shader))

    // Spirv reflection
    SpvReflectShaderModule vertex_module, fragment_module;
    SPIRV_REFLECT_CHECK(spvReflectCreateShaderModule(
        vertex_src.size(), reinterpret_cast<const uint32_t*>(vertex_src.data()), &vertex_module))
    SPIRV_REFLECT_CHECK(spvReflectCreateShaderModule(
        fragment_src.size(), reinterpret_cast<const uint32_t*>(fragment_src.data()), &fragment_module))

    PS_ASSERT(static_cast<VkShaderStageFlagBits>(vertex_module.shader_stage) == VK_SHADER_STAGE_VERTEX_BIT,
              "Vertex stage does not match");
    PS_ASSERT(static_cast<VkShaderStageFlagBits>(fragment_module.shader_stage) == VK_SHADER_STAGE_FRAGMENT_BIT,
              "Fragment stage does not match");

    // Retrieve SPIR-V info
    retrieve_vertex_input_info(vertex_module);

    retrieve_descriptor_sets_info(vertex_module, fragment_module);
    retrieve_push_constants(vertex_module, fragment_module);

    // Cleanup
    spvReflectDestroyShaderModule(&vertex_module);
    spvReflectDestroyShaderModule(&fragment_module);
}

VulkanShader::~VulkanShader() {
    // Destroy shader modules
    vkDestroyShaderModule(VulkanContext::device->handle(), m_vertex_shader, nullptr);
    vkDestroyShaderModule(VulkanContext::device->handle(), m_fragment_shader, nullptr);
}

VkPipelineShaderStageCreateInfo VulkanShader::get_vertex_create_info() const {
    VkPipelineShaderStageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    create_info.module = m_vertex_shader;
    create_info.pName = "main";

    return create_info;
}

VkPipelineShaderStageCreateInfo VulkanShader::get_fragment_create_info() const {
    VkPipelineShaderStageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    create_info.module = m_fragment_shader;
    create_info.pName = "main";

    return create_info;
}

std::vector<char> VulkanShader::read_shader_file(const std::string& path) const {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    PS_ASSERT(file.is_open(), "Failed to open shader module file: {}", path)

    const auto size = (uint32_t)file.tellg();
    std::vector<char> content(size);

    file.seekg(0);
    file.read(content.data(), size);

    file.close();

    return content;
}

void VulkanShader::retrieve_vertex_input_info(const SpvReflectShaderModule& module) {
    // Input variables
    uint32_t input_variables_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumerateInputVariables(&module, &input_variables_count, nullptr))

    std::vector<SpvReflectInterfaceVariable*> input_variables(input_variables_count);
    SPIRV_REFLECT_CHECK(spvReflectEnumerateInputVariables(&module, &input_variables_count, input_variables.data()))

    const auto is_not_built_in = [](const SpvReflectInterfaceVariable* variable) {
        return static_cast<uint32_t>(variable->built_in) == std::numeric_limits<uint32_t>::max();
    };

    std::vector<SpvReflectInterfaceVariable*> non_builtin_variables;
    std::ranges::copy_if(input_variables, std::back_inserter(non_builtin_variables), is_not_built_in);
    if (non_builtin_variables.empty())
        return;

    m_binding_description = VkVertexInputBindingDescription{};
    m_binding_description->binding = 0;
    m_binding_description->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    uint32_t stride = 0;
    for (const auto* input_var : non_builtin_variables) {
        const auto format = static_cast<VkFormat>(input_var->format);

        VkVertexInputAttributeDescription description{};
        description.binding = 0;
        description.location = input_var->location;
        description.format = format;
        description.offset = stride;

        m_attribute_descriptions.push_back(description);

        stride += VulkanUtils::get_format_size(format);
    }

    m_binding_description->stride = stride;
}

void VulkanShader::retrieve_descriptor_sets_info(const SpvReflectShaderModule& vertex_module,
                                                 const SpvReflectShaderModule& fragment_module) {
    // Vertex descriptor sets
    uint32_t vertex_set_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&vertex_module, &vertex_set_count, nullptr))

    std::vector<SpvReflectDescriptorSet*> vertex_descriptor_sets(vertex_set_count);
    SPIRV_REFLECT_CHECK(
        spvReflectEnumerateDescriptorSets(&vertex_module, &vertex_set_count, vertex_descriptor_sets.data()))

    // Fragment descriptor sets
    uint32_t fragment_set_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&fragment_module, &fragment_set_count, nullptr))

    std::vector<SpvReflectDescriptorSet*> fragment_descriptor_sets(fragment_set_count);
    SPIRV_REFLECT_CHECK(
        spvReflectEnumerateDescriptorSets(&fragment_module, &fragment_set_count, fragment_descriptor_sets.data()))

    constexpr auto max_descriptor_set = 4;
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> set_bindings(max_descriptor_set);

    const auto retrieve_set_bindings =
        [&set_bindings, max_descriptor_set](const std::vector<SpvReflectDescriptorSet*>& descriptor_sets,
                                            VkShaderStageFlags stage) {
            for (const auto& set_info : descriptor_sets) {
                for (uint32_t i = 0; i < set_info->binding_count; ++i) {
                    const auto* set_binding = set_info->bindings[i];

                    VkDescriptorSetLayoutBinding binding{};
                    binding.binding = set_binding->binding;
                    binding.descriptorType = static_cast<VkDescriptorType>(set_binding->descriptor_type);
                    binding.descriptorCount = set_binding->count;
                    binding.stageFlags = stage;
                    binding.pImmutableSamplers = VK_NULL_HANDLE;

                    // Don't know why, but it's done this way in the example
                    // (https://github.com/KhronosGroup/SPIRV-Reflect/blob/master/examples/main_descriptors.cpp)
                    for (uint32_t i_dim = 0; i_dim < set_binding->array.dims_count; ++i_dim)
                        binding.descriptorCount *= set_binding->array.dims[i_dim];

                    PS_ASSERT(set_info->set < max_descriptor_set,
                              "Cannot create descriptor set bigger than {}",
                              max_descriptor_set)

                    set_bindings[set_info->set].push_back(binding);
                }
            }
        };

    retrieve_set_bindings(vertex_descriptor_sets, VK_SHADER_STAGE_VERTEX_BIT);
    retrieve_set_bindings(fragment_descriptor_sets, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create descriptor set create info
    for (uint32_t i = 0; i < max_descriptor_set; ++i) {
        const auto bindings = set_bindings[i];

        if (bindings.empty())
            continue;

        VkDescriptorSetLayoutCreateInfo descriptor_set_create_info{};
        descriptor_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptor_set_create_info.pBindings = bindings.data();

        const auto layout =
            VulkanContext::descriptor_layout_cache->create_descriptor_layout(descriptor_set_create_info);
        PS_ASSERT(layout != VK_NULL_HANDLE, "Layout has null")

        m_descriptor_sets_layout.push_back(layout);
    }
}

void VulkanShader::retrieve_push_constants(const SpvReflectShaderModule& vertex_module,
                                           const SpvReflectShaderModule& fragment_module) {
    const auto retrieve_push_constant_ranges = [&](const SpvReflectShaderModule& module, VkShaderStageFlags stage) {
        uint32_t push_constant_count;
        SPIRV_REFLECT_CHECK(spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, nullptr))

        std::vector<SpvReflectBlockVariable*> push_constant_blocks(push_constant_count);
        SPIRV_REFLECT_CHECK(
            spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, push_constant_blocks.data()))

        for (const auto& push_constant_block : push_constant_blocks) {
            VkPushConstantRange range{};
            range.offset = push_constant_block->offset;
            range.size = push_constant_block->size;
            range.stageFlags = stage;

            m_push_constant_ranges.push_back(range);
        }
    };

    retrieve_push_constant_ranges(vertex_module, VK_SHADER_STAGE_VERTEX_BIT);
    retrieve_push_constant_ranges(fragment_module, VK_SHADER_STAGE_FRAGMENT_BIT);
}

} // namespace Phos
