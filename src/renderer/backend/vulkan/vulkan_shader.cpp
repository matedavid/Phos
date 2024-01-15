#include "vulkan_shader.h"

#include <ranges>
#include <algorithm>
#include <fstream>
#include <spirv_reflect.h>
#include <glm/glm.hpp>

#include "vk_core.h"

#include "utility/logging.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_utils.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

#define SPIRV_REFLECT_CHECK(expression)                 \
    do {                                                \
        if (expression != SPV_REFLECT_RESULT_SUCCESS) { \
            PHOS_FAIL("Spirv-reflect call failed");     \
        }                                               \
    } while (false)

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

    VkShaderModule vertex_shader, fragment_shader;
    VK_CHECK(vkCreateShaderModule(VulkanContext::device->handle(), &vertex_create_info, nullptr, &vertex_shader));
    VK_CHECK(vkCreateShaderModule(VulkanContext::device->handle(), &fragment_create_info, nullptr, &fragment_shader));

    // Create Vertex and Fragment stage create infos
    VkPipelineShaderStageCreateInfo vertex_stage{};
    vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage.module = vertex_shader;
    vertex_stage.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_stage{};
    fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage.module = fragment_shader;
    fragment_stage.pName = "main";

    m_shader_stage_create_infos.push_back(vertex_stage);
    m_shader_stage_create_infos.push_back(fragment_stage);

    // Spirv reflection
    SpvReflectShaderModule vertex_module, fragment_module;
    SPIRV_REFLECT_CHECK(spvReflectCreateShaderModule(
        vertex_src.size(), reinterpret_cast<const uint32_t*>(vertex_src.data()), &vertex_module));
    SPIRV_REFLECT_CHECK(spvReflectCreateShaderModule(
        fragment_src.size(), reinterpret_cast<const uint32_t*>(fragment_src.data()), &fragment_module));

    PHOS_ASSERT(static_cast<VkShaderStageFlagBits>(vertex_module.shader_stage) == VK_SHADER_STAGE_VERTEX_BIT,
                "Vertex stage does not match");
    PHOS_ASSERT(static_cast<VkShaderStageFlagBits>(fragment_module.shader_stage) == VK_SHADER_STAGE_FRAGMENT_BIT,
                "Fragment stage does not match");

    // Retrieve SPIR-V info
    retrieve_vertex_input_info(vertex_module);

    retrieve_descriptor_sets_info(vertex_module, fragment_module);
    retrieve_push_constants(vertex_module, fragment_module);

    create_pipeline_layout();

    // Cleanup
    spvReflectDestroyShaderModule(&vertex_module);
    spvReflectDestroyShaderModule(&fragment_module);
}

VulkanShader::VulkanShader(const std::string& path) {
    const auto src = read_shader_file(path);

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = src.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(src.data());

    VkShaderModule shader;
    VK_CHECK(vkCreateShaderModule(VulkanContext::device->handle(), &create_info, nullptr, &shader));

    SpvReflectShaderModule reflect_module;
    SPIRV_REFLECT_CHECK(
        spvReflectCreateShaderModule(src.size(), reinterpret_cast<const uint32_t*>(src.data()), &reflect_module));

    VkPipelineShaderStageCreateInfo shader_stage{};
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = static_cast<VkShaderStageFlagBits>(reflect_module.shader_stage);
    shader_stage.module = shader;
    shader_stage.pName = "main";

    m_shader_stage_create_infos.push_back(shader_stage);

    // Retrieve SPIR-V info
    retrieve_descriptor_sets_info(reflect_module);
    retrieve_push_constants(reflect_module);

    create_pipeline_layout();

    // Cleanup
    spvReflectDestroyShaderModule(&reflect_module);
}

VulkanShader::~VulkanShader() {
    // Destroy shader modules
    for (const auto& stage_info : m_shader_stage_create_infos)
        vkDestroyShaderModule(VulkanContext::device->handle(), stage_info.module, nullptr);

    // Destroy pipeline layout
    vkDestroyPipelineLayout(VulkanContext::device->handle(), m_pipeline_layout, nullptr);
}

std::optional<VulkanDescriptorInfo> VulkanShader::descriptor_info(std::string_view name) const {
    const auto result = m_descriptor_info.find(std::string(name));
    if (result == m_descriptor_info.end())
        return {};

    return result->second;
}

std::vector<VulkanDescriptorInfo> VulkanShader::descriptors_in_set(uint32_t set) const {
    std::vector<VulkanDescriptorInfo> descriptors;
    for (const auto& [_, info] : m_descriptor_info) {
        if (info.set == set)
            descriptors.push_back(info);
    }

    return descriptors;
}

std::optional<VulkanPushConstantInfo> VulkanShader::push_constant_info(std::string_view name) const {
    const auto result = m_push_constant_info.find(std::string(name));
    if (result == m_push_constant_info.end())
        return {};

    return result->second;
}

std::vector<ShaderProperty> VulkanShader::get_shader_properties() const {
    std::vector<ShaderProperty> properties;

    const auto& descriptors = descriptors_in_set(2);
    for (const auto& info : descriptors) {
        if (info.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            properties.push_back({
                .type = ShaderProperty::Type::Texture,
                .name = info.name,
            });
        } else if (info.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            for (const auto& member : info.members) {
                ShaderProperty::Type type;
                if (member.size == sizeof(float))
                    type = ShaderProperty::Type::Float;
                else if (member.size == sizeof(glm::vec3))
                    type = ShaderProperty::Type::Vec3;
                else if (member.size == sizeof(glm::vec4))
                    type = ShaderProperty::Type::Vec4;

                const auto member_name = info.name + "." + member.name;
                properties.push_back({
                    .type = type,
                    .name = member_name,
                });
            }
        }
    }

    return properties;
}

std::vector<char> VulkanShader::read_shader_file(const std::string& path) const {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    PHOS_ASSERT(file.is_open(), "Failed to open shader module file: {}", path);

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
    SPIRV_REFLECT_CHECK(spvReflectEnumerateInputVariables(&module, &input_variables_count, nullptr));

    std::vector<SpvReflectInterfaceVariable*> input_variables(input_variables_count);
    SPIRV_REFLECT_CHECK(spvReflectEnumerateInputVariables(&module, &input_variables_count, input_variables.data()));

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

    std::ranges::sort(non_builtin_variables, [](SpvReflectInterfaceVariable* a, SpvReflectInterfaceVariable* b) {
        return a->location < b->location;
    });

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
    SPIRV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&vertex_module, &vertex_set_count, nullptr));

    std::vector<SpvReflectDescriptorSet*> vertex_descriptor_sets(vertex_set_count);
    SPIRV_REFLECT_CHECK(
        spvReflectEnumerateDescriptorSets(&vertex_module, &vertex_set_count, vertex_descriptor_sets.data()));

    // Fragment descriptor sets
    uint32_t fragment_set_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&fragment_module, &fragment_set_count, nullptr));

    std::vector<SpvReflectDescriptorSet*> fragment_descriptor_sets(fragment_set_count);
    SPIRV_REFLECT_CHECK(
        spvReflectEnumerateDescriptorSets(&fragment_module, &fragment_set_count, fragment_descriptor_sets.data()));

    // Retrieve descriptor set bindings
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> set_bindings(MAX_DESCRIPTOR_SET);

    retrieve_set_bindings(vertex_descriptor_sets, VK_SHADER_STAGE_VERTEX_BIT, set_bindings);
    retrieve_set_bindings(fragment_descriptor_sets, VK_SHADER_STAGE_FRAGMENT_BIT, set_bindings);

    // Create descriptor set create info
    for (uint32_t i = 0; i < MAX_DESCRIPTOR_SET; ++i) {
        const auto bindings = set_bindings[i];

        VkDescriptorSetLayoutCreateInfo descriptor_set_create_info{};
        descriptor_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptor_set_create_info.pBindings = bindings.data();

        const auto layout =
            VulkanContext::descriptor_layout_cache->create_descriptor_layout(descriptor_set_create_info);
        PHOS_ASSERT(layout != VK_NULL_HANDLE, "Layout has null");

        m_descriptor_set_layouts.push_back(layout);
    }
}

void VulkanShader::retrieve_descriptor_sets_info(const SpvReflectShaderModule& reflect_module) {
    // Vertex descriptor sets
    uint32_t set_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&reflect_module, &set_count, nullptr));

    std::vector<SpvReflectDescriptorSet*> descriptor_sets(set_count);
    SPIRV_REFLECT_CHECK(spvReflectEnumerateDescriptorSets(&reflect_module, &set_count, descriptor_sets.data()));

    // Retrieve descriptor set bindings
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> set_bindings(MAX_DESCRIPTOR_SET);
    retrieve_set_bindings(
        descriptor_sets, static_cast<VkShaderStageFlagBits>(reflect_module.shader_stage), set_bindings);

    // Create descriptor set create info
    for (uint32_t i = 0; i < MAX_DESCRIPTOR_SET; ++i) {
        const auto bindings = set_bindings[i];

        VkDescriptorSetLayoutCreateInfo descriptor_set_create_info{};
        descriptor_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptor_set_create_info.pBindings = bindings.data();

        const auto layout =
            VulkanContext::descriptor_layout_cache->create_descriptor_layout(descriptor_set_create_info);
        PHOS_ASSERT(layout != VK_NULL_HANDLE, "Layout has null");

        m_descriptor_set_layouts.push_back(layout);
    }
}

void VulkanShader::retrieve_set_bindings(const std::vector<SpvReflectDescriptorSet*>& descriptor_sets,
                                         VkShaderStageFlags stage,
                                         std::vector<std::vector<VkDescriptorSetLayoutBinding>>& set_bindings) {
    for (const auto& set_info : descriptor_sets) {
        for (uint32_t i = 0; i < set_info->binding_count; ++i) {
            auto* set_binding = set_info->bindings[i];

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

            PHOS_ASSERT(set_info->set < MAX_DESCRIPTOR_SET,
                        "Trying to create descriptor set with value {} but maximum set is {}",
                        set_info->set,
                        MAX_DESCRIPTOR_SET);

            set_bindings[set_info->set].push_back(binding);

            // Add to descriptor info for reference
            VulkanDescriptorInfo descriptor_info{};
            descriptor_info.name = std::string(set_binding->name);
            descriptor_info.type = static_cast<VkDescriptorType>(set_binding->descriptor_type);
            descriptor_info.stage = stage;
            descriptor_info.set = set_binding->set;
            descriptor_info.binding = set_binding->binding;
            descriptor_info.size = set_binding->block.size;

            for (uint32_t j = 0; j < set_binding->block.member_count; ++j) {
                const auto mem = set_binding->block.members[j];

                const VulkanUniformBufferMember member = {
                    .name = mem.name,
                    .size = mem.size,
                    .offset = mem.offset,
                };
                descriptor_info.members.push_back(member);
            }

            m_descriptor_info.insert({descriptor_info.name, descriptor_info});
        }
    }
}

void VulkanShader::retrieve_push_constants(const SpvReflectShaderModule& vertex_module,
                                           const SpvReflectShaderModule& fragment_module) {
    retrieve_push_constant_ranges(vertex_module, VK_SHADER_STAGE_VERTEX_BIT);
    retrieve_push_constant_ranges(fragment_module, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanShader::retrieve_push_constants(const SpvReflectShaderModule& reflect_module) {
    retrieve_push_constant_ranges(reflect_module, static_cast<VkShaderStageFlagBits>(reflect_module.shader_stage));
}

void VulkanShader::retrieve_push_constant_ranges(const SpvReflectShaderModule& module, VkShaderStageFlags stage) {
    uint32_t push_constant_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, nullptr));

    std::vector<SpvReflectBlockVariable*> push_constant_blocks(push_constant_count);
    SPIRV_REFLECT_CHECK(
        spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, push_constant_blocks.data()));

    for (const auto& push_constant_block : push_constant_blocks) {
        VkPushConstantRange range{};
        range.offset = push_constant_block->offset;
        range.size = push_constant_block->size;
        range.stageFlags = stage;

        m_push_constant_ranges.push_back(range);

        VulkanPushConstantInfo push_constant_info{};
        push_constant_info.name = push_constant_block->name;
        push_constant_info.size = push_constant_block->size;
        push_constant_info.stage = stage;

        m_push_constant_info.insert({push_constant_block->name, push_constant_info});
    }
}

void VulkanShader::create_pipeline_layout() {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(m_descriptor_set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = m_descriptor_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(m_push_constant_ranges.size());
    pipeline_layout_create_info.pPushConstantRanges = m_push_constant_ranges.data();

    VK_CHECK(vkCreatePipelineLayout(
        VulkanContext::device->handle(), &pipeline_layout_create_info, nullptr, &m_pipeline_layout));
}

} // namespace Phos
