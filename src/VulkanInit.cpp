
#include "VulkanInit.h"
#include "Main.h"

// TODO(Matt): Switch these for dynamic arrays ASAP.
#define material_count 8
#define box_count 8

// Debug callback relays messages from validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    // Relay validation messages through stderr.
    std::cerr << "Validation: " << callback_data->pMessage << std::endl;
    return VK_FALSE;
}

// Selects an image format for the swapchain surface.
static void ChooseSurfaceFormat(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Check the available surface formats.
    VkSurfaceFormatKHR *available;
    uint32_t available_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_info->physical_device, vulkan_info->surface, &available_count, nullptr);
    available = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * available_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_info->physical_device, vulkan_info->surface, &available_count, available);
    // NOTE(Matt): Undefined indicates that any format is fine.
    if (available_count == 1 && available[0].format == VK_FORMAT_UNDEFINED) {
        swapchain_info->format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        free(available);
        return;
    }
    // If our desired format is available, use that.
    for (uint32_t i = 0; i < available_count; ++i) {
        if (available[i].format == VK_FORMAT_B8G8R8A8_UNORM && available[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain_info->format = available[i];
            free(available);
            return;
        }
    }
    // Otherwise, use the first format available.
    swapchain_info->format = available[0];
    free(available);
}

// Selects a swapchain present mode. Mailbox (triple buffered) is preferred. 
static void ChoosePresentMode(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Check the available present modes.
    VkPresentModeKHR *available;
    uint32_t available_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_info->physical_device, vulkan_info->surface, &available_count, nullptr);
    available = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * available_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_info->physical_device, vulkan_info->surface, &available_count, available);
    for (uint32_t i = 0; i < available_count; ++i) {
        // Prefer mailbox, if available.
        if (available[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchain_info->present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            free(available);
            return;
        }
    }
    // If it isn't, just use FIFO.
    swapchain_info->present_mode = VK_PRESENT_MODE_FIFO_KHR;
    free(available);
}

// Selects a size for the swapchain.
static void ChooseSwapchainExtent(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Get the surface capabilities.
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_info->physical_device, vulkan_info->surface, &capabilities);
    swapchain_info->transform = capabilities.currentTransform;
    swapchain_info->image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && swapchain_info->image_count > capabilities.maxImageCount) {
        swapchain_info->image_count = capabilities.maxImageCount;
    }
    // Value 0xffffffff indicates the application sets the surface size.
    if (capabilities.currentExtent.width != 0xffffffff) {
        swapchain_info->extent = capabilities.currentExtent;
    } else {
        // TODO(Matt): Platform specific.
        uint32_t width, height;
        Win32GetSurfaceSize(&width, &height);
        swapchain_info->extent = {width, height};
    }
    // Clamp swapchain size between the min and max allowed.
    if (swapchain_info->extent.width > capabilities.maxImageExtent.width) {
        swapchain_info->extent.width = capabilities.maxImageExtent.width;
    } 
    if (swapchain_info->extent.width < capabilities.minImageExtent.width) {
        swapchain_info->extent.width = capabilities.minImageExtent.width;
    }
    if (swapchain_info->extent.height > capabilities.maxImageExtent.height) {
        swapchain_info->extent.height = capabilities.maxImageExtent.height;
    }
    if (swapchain_info->extent.height < capabilities.minImageExtent.height) {
        swapchain_info->extent.height = capabilities.minImageExtent.height;
    }
}

static void CreateInstance(VulkanInfo *vulkan_info)
{
    // If validation is enabled, create the instance with layers enabled.
    // TODO(Matt): Better handling of app/engine name and versions. ini?
    if (enable_validation)
    {
        // Check available layers.
        uint32_t available_count;
        VkLayerProperties *available;
        vkEnumerateInstanceLayerProperties(&available_count, nullptr);
        available = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * available_count);
        vkEnumerateInstanceLayerProperties(&available_count, available);
        
        // Make sure the requested layers are available.
        if (!CheckValidationLayerSupport(available, available_count)) {
            ExitWithError("Validation layers unsupported!");
        }
        free(available);
    }
    // Set up application/engine info.
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = APPLICATION_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = ENGINE_NAME;
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
    
    VK_CHECK_RESULT(vkCreateInstance(&create_info, nullptr, &vulkan_info->instance));
}

static void CreateDebugMessenger(VulkanInfo *vulkan_info)
{
    // Configure messenger to relay errors/warnings but not info messages.
    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugMessenger;
    
    VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(vulkan_info->instance, &create_info, nullptr, &vulkan_info->debug_messenger));
}

static void CreateSurface(VulkanInfo *vulkan_info)
{
    // Create the window surface, given the OS window handle.
    // TODO(Matt): Platform specific.
    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = Win32GetWindowHandle();
    create_info.hinstance = GetModuleHandle(nullptr);
    VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(vulkan_info->instance, &create_info, nullptr, &vulkan_info->surface));
}

static void ChoosePhysicalDevice(VulkanInfo *vulkan_info)
{
    uint32_t graphics_index;
    uint32_t present_index;
    bool use_shared_queue;
    
    // First, query available devices.
    uint32_t available_count = 0;
    VkPhysicalDevice *available_devices;
    vkEnumeratePhysicalDevices(vulkan_info->instance, &available_count, nullptr);
    if (available_count == 0) ExitWithError("No Vulkan-supported devices found!");
    available_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * available_count);
    vkEnumeratePhysicalDevices(vulkan_info->instance, &available_count, available_devices);
    
    // Scan the list for suitable devices.
    for (uint32_t i = 0; i < available_count; ++i) {
        VkPhysicalDevice device = available_devices[i];
        bool queues_supported = false;
        bool extensions_supported = false;
        bool swapchain_supported = false;
        
        // Check if graphics and presentation to the current surface are supported by the device queues.
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan_info->surface, &present_supported);
            if (queue_families[i].queueCount > 0 && present_supported) {
                present_index = i;
                present_found = true;
            }
        }
        
        free(queue_families);
        if (graphics_found && present_found) {
            use_shared_queue = (graphics_index == present_index);
            queues_supported = true;
        }
        else {
            continue;
        }
        
        // Check if the required device extensions are supported.
        uint32_t available_extension_count;
        VkExtensionProperties *available_extensions;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extension_count, nullptr);
        available_extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * available_extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extension_count, available_extensions);
        extensions_supported = CheckDeviceExtensionSupport(available_extensions, available_extension_count);
        free(available_extensions);
        if (!extensions_supported)
            continue;
        
        // Check if the swapchain support is suitable.
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *formats;
        VkPresentModeKHR *present_modes;
        uint32_t format_count;
        uint32_t present_mode_count;
        
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan_info->surface, &capabilities);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_info->surface, &format_count, nullptr);
        formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_info->surface, &format_count, formats);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_info->surface, &present_mode_count, nullptr);
        present_modes = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_info->surface, &present_mode_count, present_modes);
        // TODO(Matt): Use a more sophisticated setup than "can present".
        swapchain_supported = (format_count > 0 && present_mode_count > 0);
        free(formats);
        free(present_modes);
        if (!swapchain_supported)
            continue;
        // TODO(Matt): Move required device features (anisotropic filtering
        // sample shading) somewhere else. Maybe don't use if unsupported?
        VkPhysicalDeviceFeatures supported_features;
        vkGetPhysicalDeviceFeatures(device, &supported_features);
        if (queues_supported && extensions_supported && swapchain_supported && supported_features.samplerAnisotropy && supported_features.sampleRateShading) {
            // Query for the max MSAA sample count.
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            VkSampleCountFlags counts = (uint32_t)fmin((float)properties.limits.framebufferColorSampleCounts,(float) properties.limits.framebufferDepthSampleCounts);
            VkSampleCountFlagBits samples;
            if (counts & VK_SAMPLE_COUNT_64_BIT) {
                samples = VK_SAMPLE_COUNT_64_BIT;
            } else if (counts & VK_SAMPLE_COUNT_32_BIT) {
                samples = VK_SAMPLE_COUNT_32_BIT;
            } else if (counts & VK_SAMPLE_COUNT_16_BIT) {
                samples = VK_SAMPLE_COUNT_16_BIT;
            } else if (counts & VK_SAMPLE_COUNT_8_BIT) {
                samples = VK_SAMPLE_COUNT_8_BIT;
            } else if (counts & VK_SAMPLE_COUNT_4_BIT) {
                samples = VK_SAMPLE_COUNT_4_BIT;
            } else if (counts & VK_SAMPLE_COUNT_2_BIT) {
                samples = VK_SAMPLE_COUNT_2_BIT;
            } else {
                samples = VK_SAMPLE_COUNT_1_BIT;
            }
            
            // Use this device.
            vulkan_info->physical_device = device;
            vulkan_info->msaa_samples = samples;
            vulkan_info->graphics_index = graphics_index;
            vulkan_info->present_index = present_index;
            vulkan_info->use_shared_queue = use_shared_queue;
            break;
        }
    }
    free(available_devices);
    if (vulkan_info->physical_device == VK_NULL_HANDLE) {
        ExitWithError("Found a Vulkan-compatible device, but none which meet this application's requirements!");
    }
}

static void CreateLogicalDevice(VulkanInfo *vulkan_info)
{
    // Setup queue create info. 
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo *create_infos;
    uint32_t family_count = (vulkan_info->use_shared_queue) ? 1 : 2;
    create_infos = (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * family_count);
    for (uint32_t i = 0; i < family_count; ++i) {
        create_infos[i] = {};
        create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_infos[i].queueFamilyIndex = (i == 0) ? vulkan_info->graphics_index : vulkan_info->present_index;
        create_infos[i].queueCount = 1;
        create_infos[i].pQueuePriorities = &queue_priority;
    }
    
    // Setup device features and extensions.
    VkPhysicalDeviceFeatures features = {};
    features.samplerAnisotropy = VK_TRUE;
    features.sampleRateShading = VK_TRUE;
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
    }
    else {
        create_info.enabledLayerCount = 0;
    }
    
    // Create the device.
    VK_CHECK_RESULT(vkCreateDevice(vulkan_info->physical_device, &create_info, nullptr, &vulkan_info->logical_device));
    vkGetDeviceQueue(vulkan_info->logical_device, vulkan_info->graphics_index, 0, &vulkan_info->graphics_queue);
    if (!vulkan_info->use_shared_queue) {
        vkGetDeviceQueue(vulkan_info->logical_device, vulkan_info->present_index, 0, &vulkan_info->present_queue);
    }
    free(create_infos);
}

// TODO(Matt): Support swapchain creation while the old one still exists.
// Defer destruction of the old one to speed it up.
static void CreateSwapchain(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    ChooseSurfaceFormat(vulkan_info, swapchain_info);
    ChoosePresentMode(vulkan_info, swapchain_info);
    ChooseSwapchainExtent(vulkan_info, swapchain_info);
    
    // Create the swapchain from the given info.
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vulkan_info->surface;
    
    create_info.minImageCount = swapchain_info->image_count;
    create_info.imageFormat = swapchain_info->format.format;
    create_info.imageColorSpace = swapchain_info->format.colorSpace;
    create_info.imageExtent = swapchain_info->extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (vulkan_info->use_shared_queue) {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        uint32_t queue_indices[] = {vulkan_info->graphics_index, vulkan_info->present_index};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_indices;
    }
    create_info.preTransform = swapchain_info->transform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = swapchain_info->present_mode;
    create_info.clipped = VK_TRUE;
    VK_CHECK_RESULT(vkCreateSwapchainKHR(vulkan_info->logical_device, &create_info, nullptr, &swapchain_info->swapchain));
    
    // Get swapchain images and image views.
    vkGetSwapchainImagesKHR(vulkan_info->logical_device, swapchain_info->swapchain, &swapchain_info->image_count, nullptr);
    swapchain_info->images = (VkImage *)malloc(sizeof(VkImage) * swapchain_info->image_count);
    vkGetSwapchainImagesKHR(vulkan_info->logical_device, swapchain_info->swapchain, &swapchain_info->image_count, swapchain_info->images);
    swapchain_info->imageviews = (VkImageView *)malloc(sizeof(VkImageView) * swapchain_info->image_count);
    
    for (uint32_t i = 0; i < swapchain_info->image_count; ++i) {
        swapchain_info->imageviews[i] = CreateImageView(vulkan_info, swapchain_info->images[i], swapchain_info->format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

static void CreateRenderpass(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Setup attachments (color, depth/stencil, resolve).
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = swapchain_info->format.format;
    color_attachment.samples = vulkan_info->msaa_samples;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference color_attach_ref = {};
    color_attach_ref.attachment = 0;
    color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // This application requires a format with stencil buffer.
    VkFormat formats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    swapchain_info->depth_format = FindSupportedFormat(vulkan_info, formats, 2, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    
    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = swapchain_info->depth_format;
    depth_attachment.samples = vulkan_info->msaa_samples;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depth_attach_ref = {};
    depth_attach_ref.attachment = 1;
    depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription resolve_attachment = {};
    resolve_attachment.format = swapchain_info->format.format;
    resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    resolve_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference resolve_attach_ref = {};
    resolve_attach_ref.attachment = 2;
    resolve_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // Setup subpass (just one at the moment).
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attach_ref;
    subpass.pDepthStencilAttachment = &depth_attach_ref;
    subpass.pResolveAttachments = &resolve_attach_ref;
    
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment, resolve_attachment};
    
    // Create the renderpass.
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 3;
    create_info.pAttachments = attachments;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;
    
    VK_CHECK_RESULT(vkCreateRenderPass(vulkan_info->logical_device, &create_info, nullptr, &swapchain_info->renderpass));
}

static void CreateFramebuffers(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Allocate space for framebuffers.
    swapchain_info->framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchain_info->image_count);
    
    // Create framebuffers with color and depth/stencil attachments.
    for (uint32_t i = 0; i < swapchain_info->image_count; ++i) {
        VkImageView attachments[] = {
            swapchain_info->color_image_view, swapchain_info->depth_image_view, swapchain_info->imageviews[i]};
        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = swapchain_info->renderpass;
        create_info.attachmentCount = 3;
        create_info.pAttachments = attachments;
        create_info.width = swapchain_info->extent.width;
        create_info.height = swapchain_info->extent.height;
        create_info.layers = 1;
        
        VK_CHECK_RESULT(vkCreateFramebuffer(vulkan_info->logical_device, &create_info, nullptr, &swapchain_info->framebuffers[i]));
    }
}
static void CreateCommandPools(VulkanInfo *vulkan_info)
{
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = vulkan_info->graphics_index;
    create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(vulkan_info->logical_device, &create_info, nullptr, &vulkan_info->primary_command_pool));
}

static void CreateDescriptorPools(VulkanInfo *vulkan_info, const SwapchainInfo *swapchain_info)
{
    // Setup descriptor sizes.
    VkDescriptorPoolSize uniform_size = {};
    uniform_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_size.descriptorCount = swapchain_info->image_count * box_count;
    
    VkDescriptorPoolSize sampler_size = {};
    sampler_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_size.descriptorCount = swapchain_info->image_count * box_count;
    
    VkDescriptorPoolSize pool_sizes[] = {uniform_size, sampler_size};
    
    // Create descriptor pool.
    VkDescriptorPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 2;
    create_info.pPoolSizes = pool_sizes;
    create_info.maxSets = swapchain_info->image_count * box_count;
    
    VK_CHECK_RESULT(vkCreateDescriptorPool(vulkan_info->logical_device, &create_info, nullptr, &vulkan_info->descriptor_pool));
}

static void CreateCommandBuffers(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    swapchain_info->primary_command_buffers = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * swapchain_info->image_count);
    
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = vulkan_info->primary_command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = swapchain_info->image_count;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkan_info->logical_device, &allocate_info, swapchain_info->primary_command_buffers));
}

static void CreateSyncPrimitives(VulkanInfo *vulkan_info)
{
    vulkan_info->image_available_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    vulkan_info->render_finished_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    vulkan_info->in_flight_fences = (VkFence *)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo sem_create_info = {};
    sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VK_CHECK_RESULT(vkCreateSemaphore(vulkan_info->logical_device, &sem_create_info, nullptr, &vulkan_info->image_available_semaphores[i]));
        VK_CHECK_RESULT(vkCreateSemaphore(vulkan_info->logical_device, &sem_create_info, nullptr, &vulkan_info->render_finished_semaphores[i]));
        VK_CHECK_RESULT(vkCreateFence(vulkan_info->logical_device, &fence_create_info, nullptr, &vulkan_info->in_flight_fences[i]));
    }
}

static void DestroySwapchain(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Destroy attachments.
    vkDestroyImageView(vulkan_info->logical_device, swapchain_info->color_image_view, nullptr);
    vkDestroyImage(vulkan_info->logical_device, swapchain_info->color_image, nullptr);
    vkFreeMemory(vulkan_info->logical_device, swapchain_info->color_image_memory, nullptr);
    vkDestroyImageView(vulkan_info->logical_device, swapchain_info->depth_image_view, nullptr);
    vkDestroyImage(vulkan_info->logical_device, swapchain_info->depth_image, nullptr);
    vkFreeMemory(vulkan_info->logical_device, swapchain_info->depth_image_memory, nullptr);
    
    // Destroy framebuffers.
    for (uint32_t i = 0; i < swapchain_info->image_count; ++i) {
        vkDestroyFramebuffer(vulkan_info->logical_device, swapchain_info->framebuffers[i], nullptr);
    }
    free(swapchain_info->framebuffers);
    
    // Destroy command buffers.
    vkFreeCommandBuffers(vulkan_info->logical_device, vulkan_info->primary_command_pool, swapchain_info->image_count, swapchain_info->primary_command_buffers);
    free(swapchain_info->primary_command_buffers);
    
    // Destroy pipelines.
    DestroyMaterials();
    // Destroy render pass.
    vkDestroyRenderPass(vulkan_info->logical_device, swapchain_info->renderpass, nullptr);
    
    // Destroy swapchain images.
    for (uint32_t i = 0; i < swapchain_info->image_count; ++i) {
        vkDestroyImageView(vulkan_info->logical_device, swapchain_info->imageviews[i], nullptr);
    }
    free(swapchain_info->imageviews);
    
    // Destroy swapchain.
    vkDestroySwapchainKHR(vulkan_info->logical_device, swapchain_info->swapchain, nullptr);
    free(swapchain_info->images);
}


static void CreateDepthImage(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    CreateImage(vulkan_info, swapchain_info->extent.width, swapchain_info->extent.height, swapchain_info->depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &swapchain_info->depth_image, &swapchain_info->depth_image_memory, 1, vulkan_info->msaa_samples);
    swapchain_info->depth_image_view = CreateImageView(vulkan_info, swapchain_info->depth_image, swapchain_info->depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

static void CreateColorImage(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    VkFormat format = swapchain_info->format.format;
    
    CreateImage(vulkan_info, swapchain_info->extent.width, swapchain_info->extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &swapchain_info->color_image, &swapchain_info->color_image_memory, 1, vulkan_info->msaa_samples);
    swapchain_info->color_image_view = CreateImageView(vulkan_info, swapchain_info->color_image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    
    TransitionImageLayout(vulkan_info, swapchain_info->color_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
}

void InitializeVulkan(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // TODO(Matt): Platform specific.
    Win32LoadVulkanLibrary();
    LoadVulkanGlobalFunctions();
    CreateInstance(vulkan_info);
    LoadVulkanInstanceFunctions(vulkan_info->instance);
    LoadVulkanInstanceExtensionFunctions(vulkan_info->instance);
    if (enable_validation) CreateDebugMessenger(vulkan_info);
    CreateSurface(vulkan_info);
    ChoosePhysicalDevice(vulkan_info);
    CreateLogicalDevice(vulkan_info);
    LoadVulkanDeviceFunctions(vulkan_info->logical_device);
    LoadVulkanDeviceExtensionFunctions(vulkan_info->logical_device);
    CreateSwapchain(vulkan_info, swapchain_info);
    CreateRenderpass(vulkan_info, swapchain_info);
    CreateMaterials();
    CreateCommandPools(vulkan_info);
    CreateColorImage(vulkan_info, swapchain_info);
    CreateDepthImage(vulkan_info, swapchain_info);
    CreateFramebuffers(vulkan_info, swapchain_info);
    CreateDescriptorPools(vulkan_info, swapchain_info);
    CreateCommandBuffers(vulkan_info, swapchain_info);
    CreateSyncPrimitives(vulkan_info);
}

// TODO(Matt): Need to recreate screen-space stuff here, like UI models.
void RecreateSwapchain(const VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    vkDeviceWaitIdle(vulkan_info->logical_device);
    DestroySwapchain(vulkan_info, swapchain_info);
    ChooseSwapchainExtent(vulkan_info, swapchain_info);
    // TODO(Matt): Platform specific.
    while (swapchain_info->extent.width == 0 || swapchain_info->extent.height == 0) {
        Win32PollEvents();
        ChooseSwapchainExtent(vulkan_info, swapchain_info);
    }
    CreateSwapchain(vulkan_info, swapchain_info);
    CreateRenderpass(vulkan_info, swapchain_info);
    CreateMaterials();
    CreateColorImage(vulkan_info, swapchain_info);
    CreateDepthImage(vulkan_info, swapchain_info);
    CreateFramebuffers(vulkan_info, swapchain_info);
    CreateCommandBuffers(vulkan_info, swapchain_info);
}

void ShutdownVulkan(VulkanInfo *vulkan_info, SwapchainInfo *swapchain_info)
{
    // Wait for the device to finish any current work.
    vkDeviceWaitIdle(vulkan_info->logical_device);
    // Destroy scene objects.
    DestroyScene();
    // Destroy the swapchain.
    DestroySwapchain(vulkan_info, swapchain_info);
    // Destroy descriptor pool and layout.
    vkDestroyDescriptorPool(vulkan_info->logical_device, vulkan_info->descriptor_pool, nullptr);
    
    // Destroy sync objects.
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkan_info->logical_device, vulkan_info->render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(vulkan_info->logical_device, vulkan_info->image_available_semaphores[i], nullptr);
        vkDestroyFence(vulkan_info->logical_device, vulkan_info->in_flight_fences[i], nullptr);
    }
    free(vulkan_info->image_available_semaphores);
    free(vulkan_info->render_finished_semaphores);
    free(vulkan_info->in_flight_fences);
    
    // Destroy command pools.
    vkDestroyCommandPool(vulkan_info->logical_device, vulkan_info->primary_command_pool, nullptr);
    
    // Destroy logical device.
    vkDestroyDevice(vulkan_info->logical_device, nullptr);
    
    // Destroy validation callback, if enabled.
#ifndef NDEBUG
    if (enable_validation)
    {
        vkDestroyDebugUtilsMessengerEXT(vulkan_info->instance, vulkan_info->debug_messenger, nullptr);
    }
#endif
    
    // Destroy window surface.
    vkDestroySurfaceKHR(vulkan_info->instance, vulkan_info->surface, nullptr);
    
    // Destroy Vulkan instance.
    vkDestroyInstance(vulkan_info->instance, nullptr);
    
    // Unload the Vulkan library.
    // TODO(Matt): Platform specific.
    Win32FreeVulkanLibrary();
}

void CreateBuffer(const VulkanInfo *vulkan_info, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
{
    // Create buffer object.
    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK_RESULT(vkCreateBuffer(vulkan_info->logical_device, &create_info, nullptr, &buffer));
    
    // Allocate device memory.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(vulkan_info->logical_device, buffer, &requirements);
    
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryType(vulkan_info, requirements.memoryTypeBits, properties);
    
    VK_CHECK_RESULT(vkAllocateMemory(vulkan_info->logical_device, &allocate_info, nullptr, &memory));
    
    // Bind memory to buffer.
    vkBindBufferMemory(vulkan_info->logical_device, buffer, memory, 0);
}

void CopyBuffer(const VulkanInfo *vulkan_info, VkBuffer source, VkBuffer destination, VkDeviceSize size)
{
    // Create and begin a transient command buffer.
    VkCommandBuffer command_buffer = BeginOneTimeCommand(vulkan_info);
    
    // Perform the copy.
    VkBufferCopy copy_region = {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);
    
    // End and destroy the command buffer.
    EndOneTimeCommand(vulkan_info, command_buffer);
}

uint32_t FindMemoryType(const VulkanInfo *vulkan_info, uint32_t type, VkMemoryPropertyFlags properties)
{
    // Check the available memory types.
    VkPhysicalDeviceMemoryProperties available_properties;
    vkGetPhysicalDeviceMemoryProperties(vulkan_info->physical_device, &available_properties);
    
    for (uint32_t i = 0; i < available_properties.memoryTypeCount; ++i)
    {
        // If one exists which satisfies the requirements, use it.
        if ((type & (1 << i)) && (available_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    // Otherwise, exit in error.
    ExitWithError("Unable to find a suitable memory type!");
    return 0;
}

VkCommandBuffer BeginOneTimeCommand(const VulkanInfo *vulkan_info)
{
    // Allocate the transient command buffer.
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = vulkan_info->primary_command_pool;
    alloc_info.commandBufferCount = 1;
    
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_info->logical_device, &alloc_info, &command_buffer);
    
    // Begin the transient command buffer.
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(command_buffer, &begin_info);
    
    return command_buffer;
}

void EndOneTimeCommand(const VulkanInfo *vulkan_info, VkCommandBuffer command_buffer)
{
    // Finish and submit the command buffer.
    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    
    vkQueueSubmit(vulkan_info->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    
    // Destroy the command buffer.
    vkQueueWaitIdle(vulkan_info->graphics_queue);
    vkFreeCommandBuffers(vulkan_info->logical_device, vulkan_info->primary_command_pool, 1, &command_buffer);
}


void CopyBufferToImage(const VulkanInfo *vulkan_info, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    // Start recording the copy command.
    VkCommandBuffer command_buffer = BeginOneTimeCommand(vulkan_info);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    
    // Submit the copy command.
    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndOneTimeCommand(vulkan_info, command_buffer);
}

void TransitionImageLayout(const VulkanInfo *vulkan_info, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_count)
{
    VkCommandBuffer command_buffer = BeginOneTimeCommand(vulkan_info);
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_count;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    // Set the aspect mask based on the old/new layout.
    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    // Select the source and destination stage based on the old/new format.
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags dest_stage;
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
    } else {
        ExitWithError("Unsupported image layout transition!");
    }
    
    // Create the pipeline barrier and submit the command.
    vkCmdPipelineBarrier(command_buffer, source_stage, dest_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    EndOneTimeCommand(vulkan_info, command_buffer);
}

void CreateImage(const VulkanInfo *vulkan_info, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory, uint32_t mip_count, VkSampleCountFlagBits samples)
{
    // Create the image object.
    VkImageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.extent.depth = 1;
    create_info.mipLevels = mip_count;
    create_info.arrayLayers = 1;
    create_info.format = format;
    create_info.tiling = tiling;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = usage;
    create_info.samples = samples;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK_RESULT(vkCreateImage(vulkan_info->logical_device, &create_info, nullptr, image));
    
    // Create the device memory for the image.
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(vulkan_info->logical_device, *image, &requirements);
    
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(vulkan_info, requirements.memoryTypeBits, properties);
    
    VK_CHECK_RESULT(vkAllocateMemory(vulkan_info->logical_device, &alloc_info, nullptr, image_memory));
    
    // Bind the image and the memory.
    vkBindImageMemory(vulkan_info->logical_device, *image, *image_memory, 0);
}

VkImageView CreateImageView(const VulkanInfo *vulkan_info, VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, uint32_t mip_count)
{
    VkImageViewCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.subresourceRange.aspectMask = aspect_mask;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = mip_count;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    VkImageView image_view;
    VK_CHECK_RESULT(vkCreateImageView(vulkan_info->logical_device, &create_info, nullptr, &image_view));
    return image_view;
}

VkFormat FindSupportedFormat(const VulkanInfo *vulkan_info, VkFormat *acceptable_formats, uint32_t acceptable_count, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    // Check the available image formats.
    VkFormatProperties properties;
    for (uint32_t i = 0; i < acceptable_count; ++i) {
        vkGetPhysicalDeviceFormatProperties(vulkan_info->physical_device, acceptable_formats[i], &properties);
        // Use linear as first choice, optimal as second.
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) return acceptable_formats[i];
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) return acceptable_formats[i];
    }
    // Otherwise, exit in error.
    ExitWithError("No acceptable format candidates found!");
    return acceptable_formats[0];
}