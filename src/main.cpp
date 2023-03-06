#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include <set>
#include <optional>
#include <limits>
#include <algorithm>
#include <fstream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() const { return graphics_family.has_value() && present_family.has_value(); }
};

QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    uint32_t i = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 presentation_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentation_support);

        if (presentation_support) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        ++i;
    }

    return indices;
}

bool check_device_extension_support(VkPhysicalDevice device,
                                    const std::vector<const char*>& device_extensions) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         available_extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    // Surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // Supported surface formats
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                             details.formats.data());
    }

    // Supported presentation modes
    uint32_t supported_modes_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &supported_modes_count, nullptr);

    if (supported_modes_count != 0) {
        details.present_modes.resize(supported_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &supported_modes_count,
                                                  details.present_modes.data());
    }

    return details;
}

VkSurfaceFormatKHR choose_swat_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
    for (const auto& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities,
                              const GLFWwindow* window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &width, &height);

    VkExtent2D actual_extend = {(uint32_t)width, (uint32_t)height};

    actual_extend.width = std::clamp(actual_extend.width, capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    actual_extend.height = std::clamp(actual_extend.height, capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);

    return actual_extend;
}

std::vector<char> read_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Error opening file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), (int64_t)fileSize);

    file.close();
    return buffer;
}

class HelloTriangleApplication {
  public:
    HelloTriangleApplication() = default;

    void run() {
        init_window();
        init_vulkan();
        main_loop();
        cleanup();
    }

  private:
    GLFWwindow* window;
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphics_queue;
    VkQueue presentation_queue;

    VkSwapchainKHR swap_chain;
    std::vector<VkImage> swap_chain_images;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;

    std::vector<VkImageView> swap_chain_image_views;

    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;

    void init_window() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

    const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    void init_vulkan() {
        create_instance();
        create_surface();
        create_physical_device();
        create_logical_device();
        create_swap_chain();
        create_image_views();
        create_render_pass();
        create_graphics_pipeline();
    }

    void create_instance() {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        // Extensions
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        if (enable_validation_layers && !check_validation_layer_support()) {
            throw std::runtime_error("Validation layers requested but not available");
        }

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
        create_info.enabledLayerCount = 0;

        if (enable_validation_layers) {
            create_info.enabledLayerCount = (uint32_t)validation_layers.size();
            create_info.ppEnabledLayerNames = validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
        }

        const VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance");
        }
    }

    void create_surface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Error creating KHR surface");
        }
    }

    void create_physical_device() {
        const auto is_device_suitable = [&]([[maybe_unused]] VkPhysicalDevice _device,
                                            VkSurfaceKHR _surface) -> bool {
            const auto indices = find_queue_families(_device, _surface);

            const bool extensions_supported =
                check_device_extension_support(_device, device_extensions);

            bool swap_chain_adequate = false;
            if (extensions_supported) {
                SwapChainSupportDetails swapChainSupport =
                    query_swap_chain_support(_device, _surface);
                swap_chain_adequate =
                    !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
            }

            return indices.is_complete() && extensions_supported && swap_chain_adequate;
        };

        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

        if (device_count == 0) {
            throw std::runtime_error("No GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

        for (const auto& _device : devices) {
            if (is_device_suitable(_device, surface)) {
                physical_device = _device;
                break;
            }
        }

        if (physical_device == VK_NULL_HANDLE) {
            throw std::runtime_error("No suitable GPU found");
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        std::cout << "Using physical device: " << properties.deviceName << "\n";
    }

    void create_logical_device() {
        QueueFamilyIndices indices = find_queue_families(physical_device, surface);

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        const auto unique_queue_families =
            std::set<uint32_t>{indices.graphics_family.value(), indices.present_family.value()};

        float queue_priority = 1.0f;
        for (const uint32_t queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = (uint32_t)queue_create_infos.size();

        device_create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
        device_create_info.ppEnabledExtensionNames = device_extensions.data();

        device_create_info.pEnabledFeatures = &device_features;

        if (enable_validation_layers) {
            device_create_info.enabledLayerCount = (uint32_t)validation_layers.size();
            device_create_info.ppEnabledLayerNames = validation_layers.data();
        } else {
            device_create_info.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Error creating logical device");
        }

        // Get queues handles
        vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
        vkGetDeviceQueue(device, indices.present_family.value(), 0, &presentation_queue);
    }

    void create_swap_chain() {
        SwapChainSupportDetails swap_chain_support =
            query_swap_chain_support(physical_device, surface);

        const VkSurfaceFormatKHR surface_format =
            choose_swat_surface_format(swap_chain_support.formats);
        const VkPresentModeKHR present_mode =
            choose_swap_present_mode(swap_chain_support.present_modes);
        const VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities, window);

        // Select minimum number of images for the swap chain
        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;

        if (swap_chain_support.capabilities.maxImageCount > 0 &&
            image_count > swap_chain_support.capabilities.maxImageCount) {
            image_count = swap_chain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swap_chain_create_info{};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = surface;

        swap_chain_create_info.minImageCount = image_count;
        swap_chain_create_info.imageFormat = surface_format.format;
        swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
        swap_chain_create_info.imageExtent = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueFamilyIndices swap_chain_indices = find_queue_families(physical_device, surface);
        const uint32_t queue_family_indices[2] = {swap_chain_indices.graphics_family.value(),
                                                  swap_chain_indices.present_family.value()};

        if (swap_chain_indices.graphics_family != swap_chain_indices.present_family) {
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount = 2;
            swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;
        } else {
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swap_chain_create_info.queueFamilyIndexCount = 0;
            swap_chain_create_info.pQueueFamilyIndices = nullptr;
        }

        swap_chain_create_info.preTransform = swap_chain_support.capabilities.currentTransform;
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = present_mode;
        swap_chain_create_info.clipped = VK_TRUE;
        swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

        // Create swap chain
        if (vkCreateSwapchainKHR(device, &swap_chain_create_info, nullptr, &swap_chain) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain");
        }

        // Retrieve swap chain images
        vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
        swap_chain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.data());

        swap_chain_image_format = surface_format.format;
        swap_chain_extent = extent;
    }

    void create_image_views() {
        swap_chain_image_views.resize(swap_chain_images.size());

        for (uint32_t i = 0; i < swap_chain_images.size(); ++i) {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = swap_chain_images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = swap_chain_image_format;

            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &image_view_create_info, nullptr,
                                  &swap_chain_image_views[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image view");
            }
        }
    }

    void create_render_pass() {
        // Attachment description
        VkAttachmentDescription color_attachment{};
        color_attachment.format = swap_chain_image_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Subpasses and attachment references
        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        // Create render pass
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass");
        }
    }

    void create_graphics_pipeline() {
        //
        // SHADER MODULES
        //
        const auto vertex_shader_code = read_file("../shaders/vert.spv");
        const auto fragment_shader_code = read_file("../shaders/frag.spv");

        const auto vertex_shader_module = create_shader_module(vertex_shader_code);
        const auto fragment_shader_module = create_shader_module(fragment_shader_code);

        VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
        vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_stage_info.module = vertex_shader_module;
        vertex_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
        fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_shader_stage_info.module = fragment_shader_module;
        fragment_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_info,
                                                           fragment_shader_stage_info};

        //
        // FIXED FUNCTIONS
        //

        // Dynamic State
        const std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                            VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = (uint32_t)dynamic_states.size();
        dynamic_state.pDynamicStates = dynamic_states.data();

        // Vertex input
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 0;
        vertex_input_info.pVertexBindingDescriptions = nullptr;
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions = nullptr;

        // Input assembly (kind of geometry to be drawn from vertices)
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        // Viewports and Scissors
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swap_chain_extent.width;
        viewport.height = (float)swap_chain_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swap_chain_extent;

        // Not using because using as dynamic state
        (void)viewport;
        (void)scissor;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;          // Optional
        multisampling.pSampleMask = nullptr;            // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE;      // Optional

        // Color blending
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f; // Optional
        color_blending.blendConstants[1] = 0.0f; // Optional
        color_blending.blendConstants[2] = 0.0f; // Optional
        color_blending.blendConstants[3] = 0.0f; // Optional

        // Pipeline Layout
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;         // Optional
        pipeline_layout_info.pushConstantRangeCount = 0;    // Optional
        pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout)) {
            throw std::runtime_error("Error creating pipeline layout");
        }

        //
        // CREATION OF GRAPHICS PIPELINE
        //
        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;

        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pDepthStencilState = nullptr;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state;

        pipeline_info.layout = pipeline_layout;

        pipeline_info.renderPass = render_pass;
        pipeline_info.subpass = 0;

        // Create current pipeline deriving from existing one
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipeline_info.basePipelineIndex = -1;              // Optional

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                      &graphics_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline");
        }

        // Destroy shader modules
        vkDestroyShaderModule(device, vertex_shader_module, nullptr);
        vkDestroyShaderModule(device, fragment_shader_module, nullptr);
    }

    bool check_validation_layer_support() {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

        for (const std::string layer_name : validation_layers) {
            bool layer_found = false;

            for (const auto& layer_properties : layers) {
                if (layer_name == std::string(layer_properties.layerName)) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                return false;
            }
        }

        return true;
    }

    VkShaderModule create_shader_module(const std::vector<char>& code) {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
            throw std::runtime_error("Could not create shader module");
        }

        return shader_module;
    }

    void main_loop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyPipeline(device, graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        vkDestroyRenderPass(device, render_pass, nullptr);

        for (const auto& imageView : swap_chain_image_views) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swap_chain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main() {
    HelloTriangleApplication app{};
    app.run();

    return 0;
}