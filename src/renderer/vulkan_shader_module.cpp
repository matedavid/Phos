#include "vulkan_shader_module.h"

#include <fstream>
#include <spirv_reflect.h>
#include <vulkan/vk_format_utils.h>

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_utils.h"

#define SPIRV_REFLECT_CHECK(expression)             \
    if (expression != SPV_REFLECT_RESULT_SUCCESS) { \
        CORE_FAIL("Spirv-reflect call failed");     \
    }

VulkanShaderModule::VulkanShaderModule(const std::string& path, Stage stage, std::shared_ptr<VulkanDevice> device)
    : m_stage(stage), m_device(std::move(device)) {
    const auto content = read_shader_file(path);

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = content.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(content.data());

    VK_CHECK(vkCreateShaderModule(m_device->handle(), &create_info, nullptr, &m_shader))

    if (m_stage == Stage::Vertex)
        retrieve_vertex_input_info(content);
}

VulkanShaderModule::~VulkanShaderModule() {
    vkDestroyShaderModule(m_device->handle(), m_shader, nullptr);
}

VkPipelineShaderStageCreateInfo VulkanShaderModule::get_shader_stage_create_info() const {
    VkPipelineShaderStageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = get_vulkan_stage(m_stage);
    create_info.module = m_shader;
    create_info.pName = "main";

    return create_info;
}

std::vector<char> VulkanShaderModule::read_shader_file(const std::string& path) const {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    CORE_ASSERT(file.is_open(), "Failed to open shader module file: {}", path)

    const auto size = (uint32_t)file.tellg();
    std::vector<char> content(size);

    file.seekg(0);
    file.read(content.data(), size);

    file.close();

    return content;
}

VkShaderStageFlagBits VulkanShaderModule::get_vulkan_stage(Stage stage) const {
    switch (stage) {
        default:
        case Stage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case Stage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
}

void VulkanShaderModule::retrieve_vertex_input_info(const std::vector<char>& content) {
    SpvReflectShaderModule module;
    SPIRV_REFLECT_CHECK(
        spvReflectCreateShaderModule(content.size(), reinterpret_cast<const uint32_t*>(content.data()), &module))

    // Input variables
    uint32_t input_variables_count;
    SPIRV_REFLECT_CHECK(spvReflectEnumerateInputVariables(&module, &input_variables_count, nullptr))

    std::vector<SpvReflectInterfaceVariable*> input_variables(input_variables_count);
    SPIRV_REFLECT_CHECK(spvReflectEnumerateInputVariables(&module, &input_variables_count, input_variables.data()))

    m_binding_description = {};
    m_binding_description.binding = 0;
    m_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    uint32_t stride = 0;
    for (const auto* input_var : input_variables) {
        const auto format = static_cast<VkFormat>(input_var->format);

        VkVertexInputAttributeDescription description{};
        description.binding = 0;
        description.location = input_var->location;
        description.format = format;
        description.offset = stride;

        m_attribute_descriptions.push_back(description);

        stride += VulkanUtils::get_format_size(format);
    }

    m_binding_description.stride = stride;

    // Cleanup
    spvReflectDestroyShaderModule(&module);
}
