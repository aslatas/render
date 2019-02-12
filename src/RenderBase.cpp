// TODO(Matt): There are a few platform specific bits lingering in here.
// Move surface creation and surface size query out.
#include "RenderBase.h"
#include "VulkanFunctions.h"
#include "MathTypes.h"
#include <cstring>
// TODO(Matt): Move Shader file read somewhere else.
#include <cstdio>
#include <iostream>
// TODO(Matt): Is chrono the best (read: lightweight) timing system?
// For Windows, what about QueryPerformanceCounter()?
#include <chrono>

static VulkanInfo vulkan_info = {};
static SwapchainInfo swapchain_info = {};
static BufferInfo buffer_info = {};

VertexX verts[4] = {};
// TODO(Matt): Do something different for these, obviously.
Vertex vertices[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
uint32_t vertex_count = ARRAYSIZE(vertices);
uint16_t indices[] = {
    0, 1, 2, 2, 3, 0
};
uint32_t index_count = ARRAYSIZE(indices);

char *ReadShaderFile(char *path, uint32_t *length)
{
    FILE * file = fopen (path, "rb");
    if (!file) return nullptr;
    fseek (file, 0, SEEK_END);
    *length = ftell(file);
    fseek (file, 0, SEEK_SET);
    char *buffer = (char *)malloc(*length);
    fread (buffer, 1, *length, file);
    fclose (file);
    return buffer;
}

void InitializeVulkan()
{
    verts[0].position = {-0.5f, -0.5f, 0.0f};
    verts[1].position = { 0.5f, -0.5f, 0.0f};
    verts[2].position = { 0.5f,  0.5f, 0.0f};
    verts[3].position = {-0.5f,  0.5f, 0.0f};
    verts[0].color = {1.0f, 0.0f, 0.0f};
    verts[1].color = {0.0f, 1.0f, 0.0f};
    verts[2].color = {0.0f, 0.0f, 1.0f};
    verts[3].color = {1.0f, 1.0f, 1.0f};
    
    // TODO(Matt): Platform specific.
    Win32LoadVulkanLibrary();
    LoadVulkanGlobalFunctions();
    CreateInstance();
    LoadVulkanInstanceFunctions(vulkan_info.instance);
    LoadVulkanInstanceExtensionFunctions(vulkan_info.instance);
    if (enable_validation) CreateDebugMessenger();
    CreateSurface();
    ChoosePhysicalDevice();
    CreateLogicalDevice();
    LoadVulkanDeviceFunctions(vulkan_info.logical_device);
    LoadVulkanDeviceExtensionFunctions(vulkan_info.logical_device);
    CreateSwapchain();
    CreateImageviews();
    CreateRenderpass();
    CreateDescriptorSetLayout();
    CreatePipeline();
    CreateFramebuffers();
    CreateCommandPool();
    // TODO(Matt): Find a different solution for buffer allocation.
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncPrimitives();
}

void CreateInstance()
{
    // TODO(Matt): Better handling of app/engine name and versions. ini?
    if (enable_validation) {
        uint32_t available_count;
        VkLayerProperties *available;
        vkEnumerateInstanceLayerProperties(&available_count, nullptr);
        available = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * available_count);
        vkEnumerateInstanceLayerProperties(&available_count, available);
        if (!CheckValidationLayerSupport(available, available_count)) {
            std::cerr << "Validation layers unsupported!" << std::endl;
            exit(EXIT_FAILURE);
        }
        free(available);
    }
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Rendering Prototype";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "NYCE";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = instance_extension_count;
    create_info.ppEnabledExtensionNames = instance_extensions;
    
    if (enable_validation) {
        create_info.enabledLayerCount = validation_layer_count;
        create_info.ppEnabledLayerNames = validation_layers;
    } else {
        create_info.enabledLayerCount = 0;
    }
    
    if (vkCreateInstance(&create_info, nullptr, &vulkan_info.instance) != VK_SUCCESS) {
        std::cerr << "Unable to create Vulkan Instance!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
}

// Debug callback relays messages from validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
    std::cerr << "validation: " << callback_data->pMessage << std::endl;
    return VK_FALSE;
}

void CreateDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugMessenger;
    
    if (vkCreateDebugUtilsMessengerEXT(vulkan_info.instance, &create_info, nullptr, &vulkan_info.debug_messenger) != VK_SUCCESS) {
        std::cerr << "Unable to create debug messenger!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// TODO(Matt): Platform specific.
void CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = Win32GetWindowHandle();
    create_info.hinstance = GetModuleHandle(nullptr);
    
    if (vkCreateWin32SurfaceKHR(vulkan_info.instance, &create_info, nullptr, &vulkan_info.surface) != VK_SUCCESS) {
        std::cerr << "Unable to create window surface!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ChoosePhysicalDevice()
{
    uint32_t graphics_index;
    uint32_t present_index;
    bool use_shared_queue;
    
    // First, query available devices.
    uint32_t available_count = 0;
    VkPhysicalDevice *available_devices;
    vkEnumeratePhysicalDevices(vulkan_info.instance, &available_count, nullptr);
    
    if (available_count == 0) {
        std::cerr << "No Vulkan-supported devices found!" << std::endl;
        exit(EXIT_FAILURE);
    }
    available_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * available_count);
    vkEnumeratePhysicalDevices(vulkan_info.instance, &available_count, available_devices);
    // Scan the list for suitable devices.
    for (uint32_t i = 0; i < available_count; ++i) {
        VkPhysicalDevice device = available_devices[i];
        bool queues_supported = false;
        bool extensions_supported = false;
        bool swapchain_supported = false;
        
        // Check if queues are supported.
        VkQueueFamilyProperties *queue_families;
        uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
        queue_families = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, queue_families);
        
        bool graphics_found = false;
        bool present_found = false;
        for (uint32_t i = 0; i < family_count; ++i) {
            if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_index = i;
                graphics_found = true;
            }
            VkBool32 present_supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan_info.surface, &present_supported);
            if (queue_families[i].queueCount > 0 && present_supported) {
                present_index = i;
                present_found = true;
            }
        }
        
        free(queue_families);
        if (graphics_found && present_found) {
            use_shared_queue = (graphics_index == present_index);
            queues_supported = true;
        } else {
            continue;
        }
        
        // Check if device extensions are supported.
        uint32_t available_extension_count;
        VkExtensionProperties *available_extensions;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extension_count, nullptr);
        available_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * available_extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extension_count, available_extensions);
        extensions_supported = CheckDeviceExtensionSupport(available_extensions, available_extension_count);
        free(available_extensions);
        if (!extensions_supported) continue;
        
        // Check if the swapchain support is suitable.
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *formats;
        VkPresentModeKHR *present_modes;
        uint32_t format_count;
        uint32_t present_mode_count;
        
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan_info.surface, &capabilities);
        
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_info.surface, &format_count, nullptr);
        formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_info.surface, &format_count, formats);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_info.surface, &present_mode_count, nullptr);
        present_modes = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_info.surface, &present_mode_count, present_modes);
        // TODO(Matt): Use a more sophisticated setup than "can present".
        swapchain_supported = (format_count > 0 && present_mode_count > 0);
        free(formats);
        free(present_modes);
        if (!swapchain_supported) continue;
        
        if (queues_supported && extensions_supported && swapchain_supported) {
            vulkan_info.physical_device = device;
            vulkan_info.graphics_index = graphics_index;
            vulkan_info.present_index = present_index;
            vulkan_info.use_shared_queue = use_shared_queue;
            break;
        }
    }
    
    free(available_devices);
    if (vulkan_info.physical_device == VK_NULL_HANDLE) {
        std::cerr << "Found a Vulkan-compatible device, but none which meet this program's requirements!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
}

void CreateLogicalDevice()
{
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo *create_infos;
    uint32_t family_count = (vulkan_info.use_shared_queue) ? 1 : 2;
    create_infos = (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * family_count);
    for (uint32_t i = 0; i < family_count; ++i) {
        create_infos[i] = {};
        create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_infos[i].queueFamilyIndex = (i == 0) ? vulkan_info.graphics_index : vulkan_info.present_index;
        create_infos[i].queueCount = 1;
        create_infos[i].pQueuePriorities = &queue_priority;
    }
    
    VkPhysicalDeviceFeatures features = {};
    
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = family_count;
    create_info.pQueueCreateInfos = create_infos;
    create_info.pEnabledFeatures = &features;
    create_info.enabledExtensionCount = device_extension_count;
    create_info.ppEnabledExtensionNames = device_extensions;
    if (enable_validation) {
        create_info.enabledLayerCount = validation_layer_count;
        create_info.ppEnabledLayerNames = validation_layers;
    } else {
        create_info.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(vulkan_info.physical_device, &create_info, nullptr, &vulkan_info.logical_device) != VK_SUCCESS) {
        std::cout << "Unable to create logical device!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkGetDeviceQueue(vulkan_info.logical_device, vulkan_info.graphics_index, 0, &vulkan_info.graphics_queue);
    if (!vulkan_info.use_shared_queue) {
        vkGetDeviceQueue(vulkan_info.logical_device, vulkan_info.present_index, 0, &vulkan_info.present_queue);
    }
    free(create_infos);
}

static void ChooseSurfaceFormat()
{
    VkSurfaceFormatKHR *available;
    uint32_t available_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, nullptr);
    available = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * available_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, available);
    
    if (available_count == 1 && available[0].format == VK_FORMAT_UNDEFINED) {
        swapchain_info.format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        free(available);
        return;
    }
    
    for (uint32_t i = 0; i < available_count; ++i) {
        if (available[i].format == VK_FORMAT_B8G8R8A8_UNORM && available[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain_info.format = available[i];
            free(available);
            return;
        }
    }
    swapchain_info.format = available[0];
    free(available);
}

static void ChoosePresentMode()
{
    VkPresentModeKHR *available;
    uint32_t available_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, nullptr);
    available = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * available_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, available);
    for (uint32_t i = 0; i < available_count; ++i) {
        if (available[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchain_info.present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            free(available);
            return;
        }
    }
    swapchain_info.present_mode = VK_PRESENT_MODE_FIFO_KHR;
    free(available);
}

static void ChooseSwapchainExtent()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_info.physical_device, vulkan_info.surface, &capabilities);
    swapchain_info.transform = capabilities.currentTransform;
    swapchain_info.image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && swapchain_info.image_count > capabilities.maxImageCount) {
        swapchain_info.image_count = capabilities.maxImageCount;
    }
    if (capabilities.currentExtent.width != 0xffffffff) {
        swapchain_info.extent = capabilities.currentExtent;
    } else {
        // TODO(Matt): Platform specific.
        uint32_t width, height;
        Win32GetSurfaceSize(&width, &height);
        VkExtent2D extent = {width, height};
        if (extent.width > capabilities.maxImageExtent.width) {
            extent.width = capabilities.maxImageExtent.width;
        }
        if (extent.width < capabilities.minImageExtent.width) {
            extent.width = capabilities.minImageExtent.width;
        }
        if (extent.height > capabilities.maxImageExtent.height) {
            extent.height = capabilities.maxImageExtent.height;
        }
        if (extent.height < capabilities.minImageExtent.height) {
            extent.height = capabilities.minImageExtent.height;
        }
        
        swapchain_info.extent = extent;
    }
}

// TODO(Matt): Support swapchain creation while the old one still exists.
// Defer destruction of the old one to speed it up.
void CreateSwapchain() {
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseSwapchainExtent();
    
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vulkan_info.surface;
    
    create_info.minImageCount = swapchain_info.image_count;
    create_info.imageFormat = swapchain_info.format.format;
    create_info.imageColorSpace = swapchain_info.format.colorSpace;
    create_info.imageExtent = swapchain_info.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (vulkan_info.use_shared_queue) {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        uint32_t queue_indices[] = {vulkan_info.graphics_index, vulkan_info.present_index};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_indices;
    }
    
    create_info.preTransform = swapchain_info.transform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = swapchain_info.present_mode;
    create_info.clipped = VK_TRUE;
    
    if (vkCreateSwapchainKHR(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.swapchain) != VK_SUCCESS) {
        std::cerr << "Unable to create swapchain!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkGetSwapchainImagesKHR(vulkan_info.logical_device, swapchain_info.swapchain, &swapchain_info.image_count, nullptr);
    swapchain_info.images = (VkImage *)malloc(sizeof(VkImage) * swapchain_info.image_count);
    vkGetSwapchainImagesKHR(vulkan_info.logical_device, swapchain_info.swapchain, &swapchain_info.image_count, swapchain_info.images);
}

void CreateImageviews()
{
    swapchain_info.imageviews = (VkImageView *)malloc(sizeof(VkImageView) * swapchain_info.image_count);
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchain_info.images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swapchain_info.format.format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.imageviews[i]) != VK_SUCCESS) {
            std::cerr << "Unable to create imageviews!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void CreateRenderpass() {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = swapchain_info.format.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;
    
    if (vkCreateRenderPass(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.renderpass) != VK_SUCCESS) {
        std::cerr << "Unable to create renderpass!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorCount = 1;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.pImmutableSamplers = nullptr;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    
    VkDescriptorSetLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &layout_binding;
    
    if (vkCreateDescriptorSetLayout(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.descriptor_set_layout) != VK_SUCCESS) {
        std::cerr << "Unable to create descriptor set layout!" << std::endl;
        exit(EXIT_FAILURE);
    }
}


static VkShaderModule CreateShaderModule(char *code, uint32_t length) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = length;
    create_info.pCode = reinterpret_cast<uint32_t *>(code);
    
    VkShaderModule module;
    if (vkCreateShaderModule(vulkan_info.logical_device, &create_info, nullptr, &module) != VK_SUCCESS) {
        std::cerr << "Unable to create shader module!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return module;
}

// TODO(Matt): Rework shader loading/swapping once geometry is sorted.
void CreatePipeline() {
    uint32_t vert_length;
    uint32_t frag_length;
    char *vert_code = ReadShaderFile("shaders/vert.spv", &vert_length);
    char *frag_code = ReadShaderFile("shaders/frag.spv", &frag_length);
    if (vert_code == nullptr || frag_code == nullptr) {
        std::cerr << "Unable to read shader files!" << std::endl;
        exit(EXIT_FAILURE);
    }
    VkShaderModule vert_module = CreateShaderModule(vert_code, vert_length);
    VkShaderModule frag_module = CreateShaderModule(frag_code, frag_length);
    
    VkPipelineShaderStageCreateInfo vert_create_info = {};
    vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.module = vert_module;
    vert_create_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo frag_create_info = {};
    frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.module = frag_module;
    frag_create_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo stages[] = {vert_create_info, frag_create_info};
    
    VkPipelineVertexInputStateCreateInfo input_create_info = {};
    input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VkVertexInputBindingDescription binding_description  = {};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    VkVertexInputAttributeDescription attribute_descriptions[2];
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset = 0;
    
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = sizeof(glm::vec2);
    
    input_create_info.vertexBindingDescriptionCount = 1;
    input_create_info.vertexAttributeDescriptionCount = 2;
    input_create_info.pVertexBindingDescriptions = &binding_description;
    input_create_info.pVertexAttributeDescriptions = attribute_descriptions;
    
    VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
    assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly_create_info.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchain_info.extent.width;
    viewport.height = (float) swapchain_info.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_info.extent;
    
    VkPipelineViewportStateCreateInfo viewport_create_info = {};
    viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_create_info.viewportCount = 1;
    viewport_create_info.pViewports = &viewport;
    viewport_create_info.scissorCount = 1;
    viewport_create_info.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo raster_create_info = {};
    raster_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_create_info.depthClampEnable = VK_FALSE;
    raster_create_info.rasterizerDiscardEnable = VK_FALSE;
    raster_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_create_info.lineWidth = 1.0f;
    raster_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_create_info.depthBiasEnable = VK_FALSE;
    
    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    VkPipelineColorBlendAttachmentState blend_state = {};
    blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_state.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo blend_create_info = {};
    blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_create_info.logicOpEnable = VK_FALSE;
    blend_create_info.logicOp = VK_LOGIC_OP_COPY;
    blend_create_info.attachmentCount = 1;
    blend_create_info.pAttachments = &blend_state;
    blend_create_info.blendConstants[0] = 0.0f;
    blend_create_info.blendConstants[1] = 0.0f;
    blend_create_info.blendConstants[2] = 0.0f;
    blend_create_info.blendConstants[3] = 0.0f;
    
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &swapchain_info.descriptor_set_layout;
    
    if (vkCreatePipelineLayout(vulkan_info.logical_device, &layout_create_info, nullptr, &swapchain_info.pipeline_layout) != VK_SUCCESS) {
        std::cerr << "Unable to create pipeline layout!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = stages;
    pipeline_create_info.pVertexInputState = &input_create_info;
    pipeline_create_info.pInputAssemblyState = &assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_create_info;
    pipeline_create_info.pRasterizationState = &raster_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pColorBlendState = &blend_create_info;
    pipeline_create_info.layout = swapchain_info.pipeline_layout;
    pipeline_create_info.renderPass = swapchain_info.renderpass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &swapchain_info.pipeline) != VK_SUCCESS) {
        std::cerr << "Unable to create pipeline!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkDestroyShaderModule(vulkan_info.logical_device, frag_module, nullptr);
    vkDestroyShaderModule(vulkan_info.logical_device, vert_module, nullptr);
}

void CreateFramebuffers()
{
    swapchain_info.framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchain_info.image_count);
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        VkImageView attachments[] = {
            swapchain_info.imageviews[i]
        };
        
        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = swapchain_info.renderpass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = attachments;
        create_info.width = swapchain_info.extent.width;
        create_info.height = swapchain_info.extent.height;
        create_info.layers = 1;
        
        if (vkCreateFramebuffer(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.framebuffers[i]) != VK_SUCCESS) {
            std::cerr << "Unable to create framebuffers!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void CreateCommandPool()
{
    
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = vulkan_info.graphics_index;
    
    if (vkCreateCommandPool(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.command_pool) != VK_SUCCESS) {
        std::cerr << "Unable to create command pool!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateVertexBuffer()
{
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void* data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices, (size_t) buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_info.vertex_buffer, buffer_info.vertex_buffer_memory);
    
    CopyBuffer(staging_buffer, buffer_info.vertex_buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateIndexBuffer()
{
    VkDeviceSize buffer_size = sizeof(indices[0]) * index_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices, (size_t) buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_info.index_buffer, buffer_info.index_buffer_memory);
    
    CopyBuffer(staging_buffer, buffer_info.index_buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateUniformBuffers()
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    buffer_info.uniform_buffers = (VkBuffer *)malloc(sizeof(VkBuffer) * swapchain_info.image_count);
    buffer_info.uniform_buffers_memory = (VkDeviceMemory *)malloc(sizeof(VkDeviceMemory) * swapchain_info.image_count);
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer_info.uniform_buffers[i], buffer_info.uniform_buffers_memory[i]);
    }
}

void CreateDescriptorPool()
{
    VkDescriptorPoolSize pool_size = {};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = swapchain_info.image_count;
    
    VkDescriptorPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;
    create_info.maxSets = swapchain_info.image_count;
    
    if (vkCreateDescriptorPool(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.descriptor_pool) != VK_SUCCESS) {
        std::cerr << "Unable to create descriptor pool!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateDescriptorSets()
{
    swapchain_info.descriptor_set_layouts = (VkDescriptorSetLayout *)malloc(sizeof(VkDescriptorSetLayout) * swapchain_info.image_count);
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        swapchain_info.descriptor_set_layouts[i] = swapchain_info.descriptor_set_layout;
    }
    
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = swapchain_info.descriptor_set_layouts;
    swapchain_info.descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * swapchain_info.image_count);
    if (vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, swapchain_info.descriptor_sets) != VK_SUCCESS) {
        std::cerr << "Unable to create descriptor sets!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        VkDescriptorBufferInfo descriptor_info = {};
        descriptor_info.buffer = buffer_info.uniform_buffers[i];
        descriptor_info.offset = 0;
        descriptor_info.range = sizeof(UniformBufferObject);
        
        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = swapchain_info.descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &descriptor_info;
        
        vkUpdateDescriptorSets(vulkan_info.logical_device, 1, &descriptor_write, 0, nullptr);
    }
}

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
{
    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(vulkan_info.logical_device, &create_info, nullptr, &buffer) != VK_SUCCESS) {
        std::cerr << "Unable to create buffer!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(vulkan_info.logical_device, buffer, &requirements);
    
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(vulkan_info.logical_device, &allocate_info, nullptr, &memory) != VK_SUCCESS) {
        std::cerr << "Unable to allocate buffer memory!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkBindBufferMemory(vulkan_info.logical_device, buffer, memory, 0);
}

void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = vulkan_info.command_pool;
    allocate_info.commandBufferCount = 1;
    
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_info.logical_device, &allocate_info, &command_buffer);
    
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(command_buffer, &begin_info);
    
    VkBufferCopy copy_region = {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);
    
    vkEndCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    
    vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan_info.graphics_queue);
    
    vkFreeCommandBuffers(vulkan_info.logical_device, vulkan_info.command_pool, 1, &command_buffer);
}

uint32_t FindMemoryType(uint32_t type, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties available_properties;
    vkGetPhysicalDeviceMemoryProperties(vulkan_info.physical_device, &available_properties);
    
    for (uint32_t i = 0; i < available_properties.memoryTypeCount; ++i) {
        if ((type & (1 << i)) && (available_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    std::cerr << "Unable to find a suitable memory type!" << std::endl;
    exit(EXIT_FAILURE);
}


void CreateCommandBuffers()
{
    swapchain_info.command_buffers = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * swapchain_info.image_count);
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = vulkan_info.command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = swapchain_info.image_count;
    
    if (vkAllocateCommandBuffers(vulkan_info.logical_device, &allocate_info, swapchain_info.command_buffers) != VK_SUCCESS) {
        std::cerr << "Unable to allocate command buffers!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        VkCommandBufferBeginInfo buffer_begin_info = {};
        buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        
        if (vkBeginCommandBuffer(swapchain_info.command_buffers[i], &buffer_begin_info) != VK_SUCCESS) {
            std::cerr << "Unable to begin command buffer recording!" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        VkRenderPassBeginInfo pass_begin_info = {};
        pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        pass_begin_info.renderPass = swapchain_info.renderpass;
        pass_begin_info.framebuffer = swapchain_info.framebuffers[i];
        pass_begin_info.renderArea.offset = {0, 0};
        pass_begin_info.renderArea.extent = swapchain_info.extent;
        
        VkClearValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
        pass_begin_info.clearValueCount = 1;
        pass_begin_info.pClearValues = &clear_color;
        
        vkCmdBeginRenderPass(swapchain_info.command_buffers[i], &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(swapchain_info.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipeline);
        
        VkBuffer vertex_buffers[] = {buffer_info.vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(swapchain_info.command_buffers[i], 0, 1, vertex_buffers, offsets);
        
        vkCmdBindIndexBuffer(swapchain_info.command_buffers[i], buffer_info.index_buffer, 0, VK_INDEX_TYPE_UINT16);
        
        vkCmdBindDescriptorSets(swapchain_info.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipeline_layout, 0, 1, &swapchain_info.descriptor_sets[i], 0, nullptr);
        
        vkCmdDrawIndexed(swapchain_info.command_buffers[i], index_count, 1, 0, 0, 0);
        
        vkCmdEndRenderPass(swapchain_info.command_buffers[i]);
        
        if (vkEndCommandBuffer(swapchain_info.command_buffers[i]) != VK_SUCCESS) {
            std::cerr << "Unable to record command buffer!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void CreateSyncPrimitives() 
{
    swapchain_info.image_available_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    swapchain_info.render_finished_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    swapchain_info.in_flight_fences = (VkFence *)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(vulkan_info.logical_device, &semaphore_create_info, nullptr, &swapchain_info.image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkan_info.logical_device, &semaphore_create_info, nullptr, &swapchain_info.render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vulkan_info.logical_device, &fence_create_info, nullptr, &swapchain_info.in_flight_fences[i]) != VK_SUCCESS) {
            std::cerr << "Unable to create synchronization objects!" << std::endl;
        }
    }
}


void DrawFrame() {
    vkWaitForFences(vulkan_info.logical_device, 1, &swapchain_info.in_flight_fences[swapchain_info.current_frame], VK_TRUE, 0xffffffffffffffff);
    
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(vulkan_info.logical_device, swapchain_info.swapchain, 0xffffffffffffffff, swapchain_info.image_available_semaphores[swapchain_info.current_frame], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cerr << "Failed to acquire swapchain image!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    UpdateUniforms(image_index);
    
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore wait_semaphores[] = {swapchain_info.image_available_semaphores[swapchain_info.current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &swapchain_info.command_buffers[image_index];
    
    VkSemaphore signal_semaphores[] = {swapchain_info.render_finished_semaphores[swapchain_info.current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    vkResetFences(vulkan_info.logical_device, 1, &swapchain_info.in_flight_fences[swapchain_info.current_frame]);
    
    if (vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, swapchain_info.in_flight_fences[swapchain_info.current_frame]) != VK_SUCCESS) {
        std::cerr << "Unable to submit draw command buffer!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[] = {swapchain_info.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    
    present_info.pImageIndices = &image_index;
    if (vulkan_info.use_shared_queue) {
        result = vkQueuePresentKHR(vulkan_info.graphics_queue, &present_info);
    } else {
        
        result = vkQueuePresentKHR(vulkan_info.present_queue, &present_info);
    }
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        std::cerr << "Unable to present swapchain image!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    swapchain_info.current_frame = (swapchain_info.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void RecreateSwapchain() {
    vkDeviceWaitIdle(vulkan_info.logical_device);
    CleanupSwapchain();
    
    CreateSwapchain();
    CreateImageviews();
    CreateRenderpass();
    CreatePipeline();
    CreateFramebuffers();
    CreateCommandBuffers();
}

void CleanupSwapchain() {
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        vkDestroyFramebuffer(vulkan_info.logical_device, swapchain_info.framebuffers[i], nullptr);
    }
    free(swapchain_info.framebuffers);
    
    vkFreeCommandBuffers(vulkan_info.logical_device, vulkan_info.command_pool, swapchain_info.image_count, swapchain_info.command_buffers);
    free(swapchain_info.command_buffers);
    vkDestroyPipeline(vulkan_info.logical_device, swapchain_info.pipeline, nullptr);
    vkDestroyPipelineLayout(vulkan_info.logical_device, swapchain_info.pipeline_layout, nullptr);
    vkDestroyRenderPass(vulkan_info.logical_device, swapchain_info.renderpass, nullptr);
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        vkDestroyImageView(vulkan_info.logical_device, swapchain_info.imageviews[i], nullptr);
    }
    free(swapchain_info.imageviews);
    
    vkDestroySwapchainKHR(vulkan_info.logical_device, swapchain_info.swapchain, nullptr);
    free(swapchain_info.images);
}

void ShutdownVulkan()
{
    // TODO(Matt): IMPORTANT: actually free memory and shutdown vulkan.
    // Right now we are letting the OS do a lot.
    vkDeviceWaitIdle(vulkan_info.logical_device);
    CleanupSwapchain();
    // TODO(Matt): Platform specific.
    Win32FreeVulkanLibrary();
}

// TODO(Matt): Figure out uniforms in general.
void UpdateUniforms(uint32_t current_image) {
    static auto start_time = std::chrono::high_resolution_clock::now();
    
    auto current_time = std::chrono::high_resolution_clock::now();
    float run_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    
    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), run_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_info.extent.width / (float) swapchain_info.extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    
    void* data;
    vkMapMemory(vulkan_info.logical_device, buffer_info.uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vulkan_info.logical_device, buffer_info.uniform_buffers_memory[current_image]);
}

bool CheckValidationLayerSupport(VkLayerProperties available[], uint32_t available_count)
{
    for (uint32_t i = 0; i < validation_layer_count; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < available_count; ++j) {
            if (strcmp(validation_layers[i], available[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool CheckInstanceExtensionSupport(VkExtensionProperties available[], uint32_t available_count)
{
    for (uint32_t i = 0; i < instance_extension_count; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < available_count; ++j) {
            if (strcmp(instance_extensions[i], available[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool CheckDeviceExtensionSupport(VkExtensionProperties available[], uint32_t available_count)
{
    for (uint32_t i = 0; i < device_extension_count; ++i) {
        bool found = false;
        for (uint32_t j = 0; j < available_count; ++j) {
            if (strcmp(device_extensions[i], available[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void LoadVulkanGlobalFunctions()
{
#define VK_GLOBAL_FUNCTION(name)                                           \
    if (!(name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name))) {         \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanInstanceFunctions(VkInstance instance)
{
#define VK_INSTANCE_FUNCTION(name)                                         \
    if (!(name = (PFN_##name)vkGetInstanceProcAddr(instance, #name))) {        \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanInstanceExtensionFunctions(VkInstance instance)
{
#define VK_INSTANCE_FUNCTION_EXT(name)                                     \
    if (!(name = (PFN_##name)vkGetInstanceProcAddr(instance, #name))) {        \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanDeviceFunctions(VkDevice device)
{
#define VK_DEVICE_FUNCTION(name)                                           \
    if (!(name = (PFN_##name)vkGetDeviceProcAddr(device, #name))) {          \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}

void LoadVulkanDeviceExtensionFunctions(VkDevice device)
{
#define VK_DEVICE_FUNCTION_EXT(name)                                       \
    if (!(name = (PFN_##name)vkGetDeviceProcAddr(device, #name))) {            \
        std::cerr << "Unable to load function: " << #name << "!" << std::endl; \
        exit(EXIT_FAILURE);                                                    \
    }
    
#include "VulkanFunctions.inl"
}
