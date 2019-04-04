
#include "VulkanInit.h"

// NOTE(Matt): I imagine once we get some threaded logic going, these will
// belong to the render thread.
// Application-wide vulkan state.
global VulkanInfo vulkan_info = {};
// Swapchain-specific vulkan state.
global SwapchainInfo swapchain_info = {};

// Debug callback relays messages from validation layers.
internal VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    // Relay validation messages through stderr.
    printf("Validation: %s\n", callback_data->pMessage);
    return VK_FALSE;
}

// Selects a size for the swapchain.
internal void ChooseSwapchainExtent(u32 *width, u32 *height)
{
    // Get the surface capabilities.
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_info.physical_device, vulkan_info.surface, &capabilities);
    swapchain_info.image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && swapchain_info.image_count > capabilities.maxImageCount) {
        swapchain_info.image_count = capabilities.maxImageCount;
    }
    // U32_MAX indicates the application sets the surface size.
    if (capabilities.currentExtent.width != U32_MAX) {
        *width = capabilities.currentExtent.width;
        *height = capabilities.currentExtent.height;
    } else {
        PlatformGetWindowSize(&global_window, width, height);
    }
    // Clamp swapchain size between the min and max allowed.
    if (*width > capabilities.maxImageExtent.width) *width = capabilities.maxImageExtent.width;
    if (*width < capabilities.minImageExtent.width) *width = capabilities.minImageExtent.width;
    if (*height > capabilities.maxImageExtent.height) *height = capabilities.maxImageExtent.height;
    if (*height < capabilities.minImageExtent.height) *height = capabilities.minImageExtent.height;
}

// TODO(Matt): Support swapchain creation while the old one still exists.
// Defer destruction of the old one to speed it up.
internal void CreateSwapchain(u32 width, u32 height)
{
    swapchain_info.extent = {width, height};
    
    // -- Choose a surface format. -- //
    {
        VkSurfaceFormatKHR *available;
        u32 available_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, nullptr);
        available = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * available_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, available);
        // NOTE(Matt): Undefined indicates that any format is fine.
        if (available_count == 1 && available[0].format == VK_FORMAT_UNDEFINED) {
            swapchain_info.format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
            free(available);
        } else {
            bool desired_is_available = false;
            // If our desired format is available, use that.
            for (u32 i = 0; i < available_count; ++i) {
                if (available[i].format == VK_FORMAT_B8G8R8A8_UNORM && available[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    swapchain_info.format = available[i];
                    desired_is_available = true;
                    free(available);
                    break;
                }
            }
            // Otherwise, use the first format available.
            if (!desired_is_available) {
                swapchain_info.format = available[0];
                free(available);
            }
        }
    }
    
    // -- Choose a presentation mode. -- //
    {
        
        // TODO(Matt): Is triple buffering always better? Would we ea
        // less CPU by doing FIFO?
        VkPresentModeKHR *available;
        u32 available_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, nullptr);
        available = (VkPresentModeKHR *)malloc(sizeof(VkPresentModeKHR) * available_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_info.physical_device, vulkan_info.surface, &available_count, available);
        bool desired_is_available = false;
        for (u32 i = 0; i < available_count; ++i) {
            // Prefer mailbox, if available.
            if (available[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchain_info.present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                free(available);
                desired_is_available = true;
                break;
            }
        }
        // If it isn't, just use FIFO.
        if (!desired_is_available) {
            swapchain_info.present_mode = VK_PRESENT_MODE_FIFO_KHR;
            free(available);
        }
    }
    
    // Create the swapchain from the given info.
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
        u32 queue_indices[] = {vulkan_info.graphics_index, vulkan_info.present_index};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_indices;
    }
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = swapchain_info.present_mode;
    // TODO(Matt): This may not be valid when we want to do post-process.
    create_info.clipped = VK_TRUE;
    VK_CHECK_RESULT(vkCreateSwapchainKHR(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.swapchain));
    // Get swapchain images and image views.
    vkGetSwapchainImagesKHR(vulkan_info.logical_device, swapchain_info.swapchain, &swapchain_info.image_count, nullptr);
    swapchain_info.images = (VkImage *)malloc(sizeof(VkImage) * swapchain_info.image_count);
    vkGetSwapchainImagesKHR(vulkan_info.logical_device, swapchain_info.swapchain, &swapchain_info.image_count, swapchain_info.images);
    swapchain_info.imageviews = (VkImageView *)malloc(sizeof(VkImageView) * swapchain_info.image_count);
    
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        swapchain_info.imageviews[i] = CreateImageView(swapchain_info.images[i], swapchain_info.format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    
    // -- Create the renderpass. -- //
    {
        // Setup attachments (color, depth/stencil, resolve).
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = swapchain_info.format.format;
        color_attachment.samples = vulkan_info.msaa_samples;
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
        swapchain_info.depth_format = FindSupportedFormat(formats, 2, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        
        VkAttachmentDescription depth_attachment = {};
        depth_attachment.format = swapchain_info.depth_format;
        depth_attachment.samples = vulkan_info.msaa_samples;
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
        resolve_attachment.format = swapchain_info.format.format;
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
        
        VK_CHECK_RESULT(vkCreateRenderPass(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.renderpass));
    }
    
    // -- Create the image attachments. -- //
    {
        // Color image.
        VkFormat format = swapchain_info.format.format;
        
        CreateImage(swapchain_info.extent.width, swapchain_info.extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &swapchain_info.color_image, &swapchain_info.color_image_memory, 1, vulkan_info.msaa_samples);
        swapchain_info.color_image_view = CreateImageView(swapchain_info.color_image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        
        TransitionImageLayout(swapchain_info.color_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
        
        // Depth image.
        CreateImage(swapchain_info.extent.width, swapchain_info.extent.height, swapchain_info.depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &swapchain_info.depth_image, &swapchain_info.depth_image_memory, 1, vulkan_info.msaa_samples);
        swapchain_info.depth_image_view = CreateImageView(swapchain_info.depth_image, swapchain_info.depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }
    
    // -- Create the framebuffers. -- //
    {
        // Allocate space for framebuffers.
        swapchain_info.framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchain_info.image_count);
        
        // Create framebuffers with color and depth/stencil attachments.
        for (u32 i = 0; i < swapchain_info.image_count; ++i) {
            VkImageView attachments[] = {
                swapchain_info.color_image_view, swapchain_info.depth_image_view, swapchain_info.imageviews[i]};
            VkFramebufferCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            create_info.renderPass = swapchain_info.renderpass;
            create_info.attachmentCount = 3;
            create_info.pAttachments = attachments;
            create_info.width = swapchain_info.extent.width;
            create_info.height = swapchain_info.extent.height;
            create_info.layers = 1;
            
            VK_CHECK_RESULT(vkCreateFramebuffer(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.framebuffers[i]));
        }
    }
    
    // -- Create the command buffers. -- //
    {
        swapchain_info.primary_command_buffers = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * swapchain_info.image_count);
        
        VkCommandBufferAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = vulkan_info.primary_command_pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = swapchain_info.image_count;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(vulkan_info.logical_device, &allocate_info, swapchain_info.primary_command_buffers));
    }
}

void DestroySwapchain()
{
    // Destroy attachments.
    vkDestroyImageView(vulkan_info.logical_device, swapchain_info.color_image_view, nullptr);
    vkDestroyImage(vulkan_info.logical_device, swapchain_info.color_image, nullptr);
    vkFreeMemory(vulkan_info.logical_device, swapchain_info.color_image_memory, nullptr);
    vkDestroyImageView(vulkan_info.logical_device, swapchain_info.depth_image_view, nullptr);
    vkDestroyImage(vulkan_info.logical_device, swapchain_info.depth_image, nullptr);
    vkFreeMemory(vulkan_info.logical_device, swapchain_info.depth_image_memory, nullptr);
    
    // Destroy framebuffers.
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        vkDestroyFramebuffer(vulkan_info.logical_device, swapchain_info.framebuffers[i], nullptr);
    }
    free(swapchain_info.framebuffers);
    
    // Destroy command buffers.
    vkFreeCommandBuffers(vulkan_info.logical_device, vulkan_info.primary_command_pool, swapchain_info.image_count, swapchain_info.primary_command_buffers);
    free(swapchain_info.primary_command_buffers);
    
    // Destroy render pass.
    vkDestroyRenderPass(vulkan_info.logical_device, swapchain_info.renderpass, nullptr);
    
    // Destroy swapchain images.
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        vkDestroyImageView(vulkan_info.logical_device, swapchain_info.imageviews[i], nullptr);
    }
    free(swapchain_info.imageviews);
    
    // Destroy swapchain.
    vkDestroySwapchainKHR(vulkan_info.logical_device, swapchain_info.swapchain, nullptr);
    free(swapchain_info.images);
}

void InitializeVulkan()
{
    // Load the vulkan library DLL and pull in global functions.
    PlatformLoadVulkanLibrary();
    LoadVulkanGlobalFunctions();
    
    // -- Create the vulkan instance. -- //
    {
        // If validation is enabled, create the instance with layers enabled.
        // TODO(Matt): Better handling of app/engine name and versions. ini?
        if (enable_validation)
        {
            // Check available layers.
            u32 available_count;
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
        create_info.enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
        create_info.ppEnabledExtensionNames = instance_extensions;
        if (enable_validation) {
            create_info.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]);
            create_info.ppEnabledLayerNames = validation_layers;
        } else {
            create_info.enabledLayerCount = 0;
        }
        
        VK_CHECK_RESULT(vkCreateInstance(&create_info, nullptr, &vulkan_info.instance));
        // Pull in instance functions from vulkan and extensions.
        LoadVulkanInstanceFunctions(vulkan_info.instance);
        LoadVulkanInstanceExtensionFunctions(vulkan_info.instance);
        // If validation is enabled, create the messenger callback.
        if (enable_validation) {
            VkDebugUtilsMessengerCreateInfoEXT create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = DebugMessenger;
            
            VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(vulkan_info.instance, &create_info, nullptr, &vulkan_info.debug_messenger));
        }
        
        // Create the window surface.
        vulkan_info.surface = PlatformCreateSurface(&global_window, vulkan_info.instance);
    }
    
    // -- Choose a physical device from those available. -- //
    {
        u32 graphics_index;
        u32 present_index;
        bool use_shared_queue;
        
        // First, query available devices.
        u32 available_count = 0;
        VkPhysicalDevice *available_devices;
        vkEnumeratePhysicalDevices(vulkan_info.instance, &available_count, nullptr);
        if (available_count == 0) ExitWithError("No Vulkan-supported devices found!");
        available_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * available_count);
        vkEnumeratePhysicalDevices(vulkan_info.instance, &available_count, available_devices);
        
        // Scan the list for suitable devices.
        for (u32 i = 0; i < available_count; ++i) {
            VkPhysicalDevice device = available_devices[i];
            bool queues_supported = false;
            bool extensions_supported = false;
            bool swapchain_supported = false;
            
            // Check if graphics and presentation to the current surface are supported by the device queues.
            VkQueueFamilyProperties *queue_families;
            u32 family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
            queue_families = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, queue_families);
            bool graphics_found = false;
            bool present_found = false;
            for (u32 i = 0; i < family_count; ++i) {
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
            }
            else {
                continue;
            }
            
            // Check if the required device extensions are supported.
            u32 available_extension_count;
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
            u32 format_count;
            u32 present_mode_count;
            
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
                VkSampleCountFlags counts = (u32)fmin((float)properties.limits.framebufferColorSampleCounts,(float) properties.limits.framebufferDepthSampleCounts);
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
                vulkan_info.physical_device = device;
                vulkan_info.msaa_samples = samples;
                vulkan_info.graphics_index = graphics_index;
                vulkan_info.present_index = present_index;
                vulkan_info.use_shared_queue = use_shared_queue;
                break;
            }
        }
        free(available_devices);
        if (vulkan_info.physical_device == VK_NULL_HANDLE) {
            ExitWithError("Found a Vulkan-compatible device, but none which meet this application's requirements!");
        }
    }
    
    // -- Create a logical device from the chosen physical device. -- //
    {
        // Setup queue create info. 
        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo *create_infos;
        u32 family_count = (vulkan_info.use_shared_queue) ? 1 : 2;
        create_infos = (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * family_count);
        for (u32 i = 0; i < family_count; ++i) {
            create_infos[i] = {};
            create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_infos[i].queueFamilyIndex = (i == 0) ? vulkan_info.graphics_index : vulkan_info.present_index;
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
        create_info.enabledExtensionCount = sizeof(device_extensions) / sizeof(device_extensions[0]);
        create_info.ppEnabledExtensionNames = device_extensions;
        if (enable_validation) {
            create_info.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]);
            create_info.ppEnabledLayerNames = validation_layers;
        }
        else {
            create_info.enabledLayerCount = 0;
        }
        
        // Create the device.
        VK_CHECK_RESULT(vkCreateDevice(vulkan_info.physical_device, &create_info, nullptr, &vulkan_info.logical_device));
        vkGetDeviceQueue(vulkan_info.logical_device, vulkan_info.graphics_index, 0, &vulkan_info.graphics_queue);
        if (!vulkan_info.use_shared_queue) {
            vkGetDeviceQueue(vulkan_info.logical_device, vulkan_info.present_index, 0, &vulkan_info.present_queue);
        }
        free(create_infos);
    }
    // Load device functions and device extension functions.
    LoadVulkanDeviceFunctions(vulkan_info.logical_device);
    LoadVulkanDeviceExtensionFunctions(vulkan_info.logical_device);
    
    // -- Create the command pools and sync primitives. -- //
    {
        VkCommandPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.queueFamilyIndex = vulkan_info.graphics_index;
        create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK_RESULT(vkCreateCommandPool(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.primary_command_pool));
        
        vulkan_info.image_available_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
        vulkan_info.render_finished_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
        vulkan_info.in_flight_fences = (VkFence *)malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
        
        VkSemaphoreCreateInfo sem_create_info = {};
        sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        
        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            VK_CHECK_RESULT(vkCreateSemaphore(vulkan_info.logical_device, &sem_create_info, nullptr, &vulkan_info.image_available_semaphores[i]));
            VK_CHECK_RESULT(vkCreateSemaphore(vulkan_info.logical_device, &sem_create_info, nullptr, &vulkan_info.render_finished_semaphores[i]));
            VK_CHECK_RESULT(vkCreateFence(vulkan_info.logical_device, &fence_create_info, nullptr, &vulkan_info.in_flight_fences[i]));
        }
    }
    
    // Create the swapchain and render pass.
    u32 width, height;
    ChooseSwapchainExtent(&width, &height);
    CreateSwapchain(width, height);
    
    // Setup descriptor sizes.
    VkDescriptorPoolSize uniform_size = {};
    uniform_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uniform_size.descriptorCount = swapchain_info.image_count * MAX_OBJECTS;
    
    VkDescriptorPoolSize sampler_size = {};
    sampler_size.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    sampler_size.descriptorCount = swapchain_info.image_count * MAX_OBJECTS * MATERIAL_SAMPLER_COUNT;
    
    VkDescriptorPoolSize texture_size = {};
    texture_size.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texture_size.descriptorCount = swapchain_info.image_count * MAX_OBJECTS * MAX_TEXTURES;
    VkDescriptorPoolSize pool_sizes[] = {uniform_size, sampler_size, texture_size};
    
    // Create descriptor pool.
    VkDescriptorPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 3;
    create_info.pPoolSizes = pool_sizes;
    create_info.maxSets = swapchain_info.image_count;
    
    VK_CHECK_RESULT(vkCreateDescriptorPool(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.descriptor_pool));
}

void RecreateSwapchain()
{
    vkDeviceWaitIdle(vulkan_info.logical_device);
    u32 width, height;
    ChooseSwapchainExtent(&width, &height);
    while (width == 0 || height == 0) {
        if (PlatformPollEvents() < 0) ShutdownVulkan();
        ChooseSwapchainExtent(&width, &height);
    }
    DestroySwapchain();
    CreateSwapchain(width, height);
}

void ShutdownVulkan()
{
    DestroySwapchain();
    
    // Destroy sync objects.
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkan_info.logical_device, vulkan_info.render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(vulkan_info.logical_device, vulkan_info.image_available_semaphores[i], nullptr);
        vkDestroyFence(vulkan_info.logical_device, vulkan_info.in_flight_fences[i], nullptr);
    }
    free(vulkan_info.image_available_semaphores);
    free(vulkan_info.render_finished_semaphores);
    free(vulkan_info.in_flight_fences);
    // Destroy descriptor pool.
    vkDestroyDescriptorPool(vulkan_info.logical_device, vulkan_info.descriptor_pool, nullptr);
    // Destroy command pools.
    vkDestroyCommandPool(vulkan_info.logical_device, vulkan_info.primary_command_pool, nullptr);
    
    // Destroy logical device.
    vkDestroyDevice(vulkan_info.logical_device, nullptr);
    
    // Destroy validation callback, if enabled.
#ifndef NDEBUG
    if (enable_validation)
    {
        vkDestroyDebugUtilsMessengerEXT(vulkan_info.instance, vulkan_info.debug_messenger, nullptr);
    }
#endif
    
    // Destroy window surface.
    vkDestroySurfaceKHR(vulkan_info.instance, vulkan_info.surface, nullptr);
    
    // Destroy Vulkan instance.
    vkDestroyInstance(vulkan_info.instance, nullptr);
    
    // Unload the Vulkan library.
    PlatformFreeVulkanLibrary();
    exit(EXIT_SUCCESS);
}

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
{
    // Create buffer object.
    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK_RESULT(vkCreateBuffer(vulkan_info.logical_device, &create_info, nullptr, &buffer));
    
    // Allocate device memory.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(vulkan_info.logical_device, buffer, &requirements);
    
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties);
    
    VK_CHECK_RESULT(vkAllocateMemory(vulkan_info.logical_device, &allocate_info, nullptr, &memory));
    
    // Bind memory to buffer.
    vkBindBufferMemory(vulkan_info.logical_device, buffer, memory, 0);
}

void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size)
{
    // Create and begin a transient command buffer.
    VkCommandBuffer command_buffer = BeginOneTimeCommand();
    
    // Perform the copy.
    VkBufferCopy copy_region = {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);
    
    // End and destroy the command buffer.
    EndOneTimeCommand(command_buffer);
}

u32 FindMemoryType(u32 type, VkMemoryPropertyFlags properties)
{
    // Check the available memory types.
    VkPhysicalDeviceMemoryProperties available_properties;
    vkGetPhysicalDeviceMemoryProperties(vulkan_info.physical_device, &available_properties);
    
    for (u32 i = 0; i < available_properties.memoryTypeCount; ++i)
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

VkCommandBuffer BeginOneTimeCommand()
{
    // Allocate the transient command buffer.
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = vulkan_info.primary_command_pool;
    alloc_info.commandBufferCount = 1;
    
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_info.logical_device, &alloc_info, &command_buffer);
    
    // Begin the transient command buffer.
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(command_buffer, &begin_info);
    
    return command_buffer;
}

void EndOneTimeCommand(VkCommandBuffer command_buffer)
{
    // Finish and submit the command buffer.
    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    
    vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    
    // Destroy the command buffer.
    vkQueueWaitIdle(vulkan_info.graphics_queue);
    vkFreeCommandBuffers(vulkan_info.logical_device, vulkan_info.primary_command_pool, 1, &command_buffer);
}

void CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height)
{
    // Start recording the copy command.
    VkCommandBuffer command_buffer = BeginOneTimeCommand();
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
    EndOneTimeCommand(command_buffer);
}

void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_count)
{
    VkCommandBuffer command_buffer = BeginOneTimeCommand();
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
    EndOneTimeCommand(command_buffer);
}

void CreateImage(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory, u32 mip_count, VkSampleCountFlagBits samples)
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
    
    VK_CHECK_RESULT(vkCreateImage(vulkan_info.logical_device, &create_info, nullptr, image));
    
    // Create the device memory for the image.
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(vulkan_info.logical_device, *image, &requirements);
    
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties);
    
    VK_CHECK_RESULT(vkAllocateMemory(vulkan_info.logical_device, &alloc_info, nullptr, image_memory));
    
    // Bind the image and the memory.
    vkBindImageMemory(vulkan_info.logical_device, *image, *image_memory, 0);
}

VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, u32 mip_count)
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
    VK_CHECK_RESULT(vkCreateImageView(vulkan_info.logical_device, &create_info, nullptr, &image_view));
    return image_view;
}

VkFormat FindSupportedFormat(VkFormat *acceptable_formats, u32 acceptable_count, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    // Check the available image formats.
    VkFormatProperties properties;
    for (u32 i = 0; i < acceptable_count; ++i) {
        vkGetPhysicalDeviceFormatProperties(vulkan_info.physical_device, acceptable_formats[i], &properties);
        // Use linear as first choice, optimal as second.
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) return acceptable_formats[i];
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) return acceptable_formats[i];
    }
    
    // Otherwise, exit in error.
    ExitWithError("No acceptable format candidates found!");
    return acceptable_formats[0];
}

VkShaderModule CreateShaderModule(char *code, u32 length)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = length;
    create_info.pCode = reinterpret_cast<u32 *>(code);
    
    VkShaderModule module;
    if (vkCreateShaderModule(vulkan_info.logical_device, &create_info, nullptr, &module) != VK_SUCCESS)
    {
        ExitWithError("Unable to create shader module!");
    }
    
    return module;
}

void CreateDescriptorSets(VkBuffer *buffers)
{
    arrsetlen(swapchain_info.descriptor_sets, swapchain_info.image_count);
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = descriptor_layout_new.descriptor_layouts;
    VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, swapchain_info.descriptor_sets));
    
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        VkDescriptorBufferInfo descriptor_info = {};
        descriptor_info.buffer = buffers[i];
        descriptor_info.offset = 0;
        descriptor_info.range = VK_WHOLE_SIZE;
        VkWriteDescriptorSet uniform_write = {};
        uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniform_write.dstSet = swapchain_info.descriptor_sets[i];
        uniform_write.dstBinding = 0;
        uniform_write.dstArrayElement = 0;
        uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        // TODO(Matt): Hardcode. Should be num render passes.
        uniform_write.descriptorCount = 1;
        uniform_write.pBufferInfo = &descriptor_info;
        vkUpdateDescriptorSets(vulkan_info.logical_device, 1, &uniform_write, 0, nullptr);
        UpdateTextureDescriptors(swapchain_info.descriptor_sets[i]);
    }
}

u32 WaitForNextImage()
{
    // Wait for an image to become available.
    vkWaitForFences(vulkan_info.logical_device, 1, &vulkan_info.in_flight_fences[swapchain_info.current_frame], VK_TRUE, U64_MAX);
    
    // Get the next available image.
    u32 image_index;
    VkResult result = vkAcquireNextImageKHR(vulkan_info.logical_device, swapchain_info.swapchain, U64_MAX, vulkan_info.image_available_semaphores[swapchain_info.current_frame], VK_NULL_HANDLE, &image_index);
    
    // If out of date, wait for idle to rebuild swapchain. Causes a hitch.
    while (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // TODO(Matt): I've never actually managed to force this case to
        // execute. Maybe by unplugging a monitor or something?
        RecreateSwapchain();
        result = vkAcquireNextImageKHR(vulkan_info.logical_device, swapchain_info.swapchain, U64_MAX, vulkan_info.image_available_semaphores[swapchain_info.current_frame], VK_NULL_HANDLE, &image_index);
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ExitWithError("Failed to acquire swapchain image!");
    }
    
    return image_index;
}

void SubmitRenderCommands(u32 image_index)
{
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore wait_semaphores[] = {vulkan_info.image_available_semaphores[swapchain_info.current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &swapchain_info.primary_command_buffers[image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vulkan_info.render_finished_semaphores[swapchain_info.current_frame];
    vkResetFences(vulkan_info.logical_device, 1, &vulkan_info.in_flight_fences[swapchain_info.current_frame]);
    VK_CHECK_RESULT(vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, vulkan_info.in_flight_fences[swapchain_info.current_frame]));
}

void PresentNextFrame(u32 image_index)
{
    // Present the newly acquired image.
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vulkan_info.render_finished_semaphores[swapchain_info.current_frame];
    VkSwapchainKHR swapchains[] = {swapchain_info.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    VkResult result = VK_SUCCESS;
    if (vulkan_info.use_shared_queue) {
        result = vkQueuePresentKHR(vulkan_info.graphics_queue, &present_info);
    } else {
        result = vkQueuePresentKHR(vulkan_info.present_queue, &present_info);
    }
    
    // If the swapchain is bad, recreate it (will likely cause frame hitch).
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // TODO(Matt): I've never actually managed to force this case.
        // Maybe by unplugging a monitor or something?
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        ExitWithError("Unable to present swapchain image!");
    }
    
    // Increment frame counter.
    swapchain_info.current_frame = (swapchain_info.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void UpdateDeviceMemory(void *data, VkDeviceSize size, VkDeviceMemory device_memory)
{
    void *mapped_memory;
    vkMapMemory(vulkan_info.logical_device, device_memory, 0, size, 0, &mapped_memory);
    memcpy(mapped_memory, data, size);
    vkUnmapMemory(vulkan_info.logical_device, device_memory);
}


void CreateDescriptorLayout(DescriptorLayout *layout)
{
    // Create immutable samplers.
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    // TODO(Matt): Hardcode.
    sampler_create_info.maxAnisotropy = 16;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = MAX_SAMPLER_LOD;
    
    for (u32 i = 0; i < MATERIAL_SAMPLER_COUNT; ++i) {
        VK_CHECK_RESULT(vkCreateSampler(vulkan_info.logical_device, &sampler_create_info, nullptr, &layout->samplers[i]));
    }
    
    // Binding 0 is uniform buffer.
    VkDescriptorSetLayoutBinding uniform_binding = {};
    uniform_binding.binding = 0;
    uniform_binding.descriptorCount = 1;
    uniform_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uniform_binding.pImmutableSamplers = nullptr;
    uniform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    // Binding 1 is the texture sampler, for now.
    VkDescriptorSetLayoutBinding sampler_binding = {};
    sampler_binding.binding = 1;
    sampler_binding.descriptorCount = MATERIAL_SAMPLER_COUNT;
    sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    sampler_binding.pImmutableSamplers = layout->samplers;
    sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    // Binding 2 is the texture library, for now.
    VkDescriptorSetLayoutBinding texture_binding = {};
    texture_binding.binding = 2;
    texture_binding.descriptorCount = MAX_TEXTURES;
    texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texture_binding.pImmutableSamplers = nullptr;
    texture_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutBinding bindings[] = {uniform_binding, sampler_binding, texture_binding};
    VkDescriptorSetLayoutCreateInfo descriptor_info = {};
    descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_info.bindingCount = 3;
    descriptor_info.pBindings = bindings;
    
    // Create descriptor set layouts.
    VkDescriptorSetLayout descriptor_layout;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vulkan_info.logical_device, &descriptor_info, nullptr, &descriptor_layout));
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        arrput(layout->descriptor_layouts, descriptor_layout);
    }
}

MaterialLayout CreateMaterialLayout()
{
    MaterialLayout layout = {};
    
    // Setup push constant block.
    VkPushConstantRange push_block = {};
    push_block.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_block.offset = 0;
    push_block.size = sizeof(PushConstantBlock);
    
    // Setup pipeline create info.
    VkPipelineLayoutCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_info.setLayoutCount = swapchain_info.image_count;
    pipeline_info.pSetLayouts = descriptor_layout_new.descriptor_layouts;
    pipeline_info.pushConstantRangeCount = 1;
    pipeline_info.pPushConstantRanges = &push_block;
    VK_CHECK_RESULT(vkCreatePipelineLayout(vulkan_info.logical_device, &pipeline_info, nullptr, &layout.pipeline_layout));
    return layout;
}

MaterialCreateInfo CreateDefaultMaterialInfo(const char *vert_file, const char *frag_file)
{
    MaterialCreateInfo result = {};
    
    result.stage_count = (frag_file) ? 2 : 1;
    result.shader_stages = (VkPipelineShaderStageCreateInfo *)malloc(sizeof(VkPipelineShaderStageCreateInfo) * result.stage_count);
    result.shader_modules = (VkShaderModule *)malloc(sizeof(VkShaderModule) * result.stage_count);
    u32 vert_length = 0;
    char *vert_code = ReadShaderFile(vert_file, &vert_length);
    if (!vert_code) {
        printf("Failed to read shader file: \"%s\"\n", vert_file);
        exit(EXIT_FAILURE);
    }
    result.shader_modules[0] = CreateShaderModule(vert_code, vert_length);
    
    VkPipelineShaderStageCreateInfo vert_create_info = {};
    vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.module = result.shader_modules[0];
    vert_create_info.pName = "main";
    result.shader_stages[0] = vert_create_info;
    if (frag_file) {
        u32 frag_length = 0;
        char *frag_code = ReadShaderFile(frag_file, &frag_length);
        if (!frag_code) {
            printf("Failed to read shader file: \"%s\"\n", frag_file);
            exit(EXIT_FAILURE);
        }
        result.shader_modules[1] = CreateShaderModule(frag_code, frag_length);
        VkPipelineShaderStageCreateInfo frag_create_info = {};
        frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_create_info.module = result.shader_modules[1];
        frag_create_info.pName = "main";
        result.shader_stages[1] = frag_create_info;
    }
    
    // TODO(Dustin): Figure out offsets
    
    result.input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    result.input_info.vertexBindingDescriptionCount = 7;
    result.input_info.vertexAttributeDescriptionCount = 7;
    
    // Binding Descriptions
    // Position
    result.binding_description[0].binding = 0;
    result.binding_description[0].stride = sizeof(glm::vec3);
    result.binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    // Normal
    result.binding_description[1].binding = 1;
    result.binding_description[1].stride = sizeof(glm::vec3);
    result.binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    // Tangent
    result.binding_description[2].binding = 2;
    result.binding_description[2].stride = sizeof(glm::vec4);
    result.binding_description[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    /// Color
    result.binding_description[3].binding = 3;
    result.binding_description[3].stride = sizeof(glm::vec4);
    result.binding_description[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    // UV0
    result.binding_description[4].binding = 4;
    result.binding_description[4].stride = sizeof(glm::vec2);
    result.binding_description[4].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    // UV1
    result.binding_description[5].binding = 5;
    result.binding_description[5].stride = sizeof(glm::vec2);
    result.binding_description[5].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    // UV2
    result.binding_description[6].binding = 6;
    result.binding_description[6].stride = sizeof(glm::vec2);
    result.binding_description[6].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    
    // Attribute Descriptions
    // Position
    result.attribute_descriptions[0].binding = result.binding_description[0].binding;
    result.attribute_descriptions[0].location = 0;
    result.attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    result.attribute_descriptions[0].offset = 0;
    
    // Normal
    result.attribute_descriptions[1].binding = result.binding_description[1].binding;
    result.attribute_descriptions[1].location = 1;
    result.attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    result.attribute_descriptions[1].offset = 0;   
    
    // Tangent
    result.attribute_descriptions[2].binding = result.binding_description[2].binding;
    result.attribute_descriptions[2].location = 2;
    result.attribute_descriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    result.attribute_descriptions[2].offset = 0;
    
    // Color
    result.attribute_descriptions[3].binding = result.binding_description[3].binding;
    result.attribute_descriptions[3].location = 3;
    result.attribute_descriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    result.attribute_descriptions[3].offset = 0;
    
    // UV0
    result.attribute_descriptions[4].binding = result.binding_description[4].binding;
    result.attribute_descriptions[4].location = 4;
    result.attribute_descriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
    result.attribute_descriptions[4].offset = 0;
    
    // UV1
    result.attribute_descriptions[5].binding = result.binding_description[5].binding;
    result.attribute_descriptions[5].location = 5;
    result.attribute_descriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
    result.attribute_descriptions[5].offset = 0;
    
    // UV2
    result.attribute_descriptions[6].binding = result.binding_description[6].binding;
    result.attribute_descriptions[6].location = 6;
    result.attribute_descriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
    result.attribute_descriptions[6].offset = 0;
    
    result.assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    result.assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    result.assembly_info.primitiveRestartEnable = VK_FALSE;
    
    result.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    result.viewport_info.viewportCount = 1;
    result.viewport_info.scissorCount = 1;
    
    
    result.raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    result.raster_info.depthClampEnable = VK_FALSE;
    result.raster_info.rasterizerDiscardEnable = VK_FALSE;
    result.raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    result.raster_info.lineWidth = 1.0f;
    result.raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
    result.raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    result.raster_info.depthBiasEnable = VK_FALSE;
    
    result.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    result.multisample_info.sampleShadingEnable = VK_TRUE;
    result.multisample_info.rasterizationSamples = vulkan_info.msaa_samples;
    
    result.blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    result.blend.blendEnable = VK_FALSE;
    
    result.blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    result.blend_info.logicOpEnable = VK_FALSE;
    result.blend_info.logicOp = VK_LOGIC_OP_COPY;
    result.blend_info.attachmentCount = 1;
    result.blend_info.blendConstants[0] = 0.0f;
    result.blend_info.blendConstants[1] = 0.0f;
    result.blend_info.blendConstants[2] = 0.0f;
    result.blend_info.blendConstants[3] = 0.0f;
    
    result.depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    result.depth_stencil.depthTestEnable = VK_TRUE;
    result.depth_stencil.depthWriteEnable = VK_TRUE;
    result.depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    result.depth_stencil.depthBoundsTestEnable = VK_FALSE;
    result.depth_stencil.minDepthBounds = 0.0f;
    result.depth_stencil.maxDepthBounds = 1.0f;
    result.depth_stencil.stencilTestEnable = VK_FALSE;
    result.depth_stencil.front = {};
    result.depth_stencil.back = {};
    result.dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
    result.dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;
    result.dynamic_state_count = 2;
    return result;
}

Material CreateMaterial(MaterialCreateInfo *material_info, VkPipelineLayout layout, u32 type, VkRenderPass renderpass, u32 sub_pass)
{
    material_info->input_info.pVertexBindingDescriptions = material_info->binding_description;
    material_info->input_info.pVertexAttributeDescriptions = material_info->attribute_descriptions;
    material_info->viewport_info.pViewports = &material_info->viewport;
    material_info->viewport_info.pScissors = &material_info->scissor;
    material_info->blend_info.pAttachments = &material_info->blend;
    VkPipelineDynamicStateCreateInfo dynamic_info = {};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_info.dynamicStateCount = material_info->dynamic_state_count;
    dynamic_info.pDynamicStates = material_info->dynamic_states;
    
    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = material_info->stage_count;
    pipeline_info.pStages = material_info->shader_stages;
    pipeline_info.pVertexInputState = &material_info->input_info;
    pipeline_info.pInputAssemblyState = &material_info->assembly_info;
    pipeline_info.pViewportState = &material_info->viewport_info;
    pipeline_info.pRasterizationState = &material_info->raster_info;
    pipeline_info.pMultisampleState = &material_info->multisample_info;
    pipeline_info.pColorBlendState = &material_info->blend_info;
    pipeline_info.pDepthStencilState = &material_info->depth_stencil;
    pipeline_info.pDynamicState = &dynamic_info;
    pipeline_info.layout = layout;
    pipeline_info.renderPass = renderpass;
    pipeline_info.subpass = sub_pass;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    
    Material material = {};
    material.type = type;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &material.pipeline));
    for (u32 i = 0; i < material_info->stage_count; ++i) {
        vkDestroyShaderModule(vulkan_info.logical_device, material_info->shader_modules[i], nullptr);
    }
    
    free(material_info->shader_stages);
    free(material_info->shader_modules);
    return material;
}

void DestroyDeviceBuffer(VkBuffer buffer)
{
    vkDestroyBuffer(vulkan_info.logical_device, buffer, nullptr);
}
void FreeDeviceMemory(VkDeviceMemory memory)
{
    vkFreeMemory(vulkan_info.logical_device, memory, nullptr);
}

void DestroyPipeline(VkPipeline pipeline)
{
    vkDestroyPipeline(vulkan_info.logical_device, pipeline, nullptr);
}

void DestroyPipelineLayout(VkPipelineLayout layout)
{
    vkDestroyPipelineLayout(vulkan_info.logical_device, layout, nullptr);
}

void DestroyDescriptorSetLayout(VkDescriptorSetLayout layout)
{
    vkDestroyDescriptorSetLayout(vulkan_info.logical_device, layout, nullptr);
}

void DestroySampler(VkSampler sampler)
{
    vkDestroySampler(vulkan_info.logical_device, sampler, nullptr);
}

void DestroyImageView(VkImageView view)
{
    vkDestroyImageView(vulkan_info.logical_device, view, nullptr);
}

void DestroyImage(VkImage image)
{
    vkDestroyImage(vulkan_info.logical_device, image, nullptr);
}

VkFormatProperties GetFormatProperties(VkFormat format)
{
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(vulkan_info.physical_device, format, &properties);
    return properties;
}

void UpdateDescriptorSet(VkWriteDescriptorSet descriptor_write)
{
    vkUpdateDescriptorSets(vulkan_info.logical_device, 1, &descriptor_write, 0, nullptr);
}

void CommandBeginRenderPass(u32 image_index)
{
    // Begin the command buffer.
    VkCommandBufferBeginInfo buffer_begin_info = {};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK_RESULT(vkBeginCommandBuffer(swapchain_info.primary_command_buffers[image_index], &buffer_begin_info));
    
    // Begin the render pass.
    VkRenderPassBeginInfo pass_begin_info = {};
    pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    pass_begin_info.renderPass = swapchain_info.renderpass;
    pass_begin_info.framebuffer = swapchain_info.framebuffers[image_index];
    pass_begin_info.renderArea.offset = {0, 0};
    pass_begin_info.renderArea.extent = swapchain_info.extent;
    VkClearValue clear_colors[2];
    clear_colors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_colors[1].depthStencil = {1.0f, 0};
    pass_begin_info.clearValueCount = 2;
    pass_begin_info.pClearValues = clear_colors;
    vkCmdBeginRenderPass(swapchain_info.primary_command_buffers[image_index], &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    
    
    // TODO(Matt): Should contain offsets of each descriptor in the buffer.
    u32 offsets[] = {0};
    vkCmdBindDescriptorSets(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material_types[0].pipeline_layout, 0, 1, &swapchain_info.descriptor_sets[image_index], 1, offsets);
}

void CommandBindPipeline(VkPipeline pipeline, u32 image_index)
{
    vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    
    VkViewport viewport = {0.0f, 0.0f, (float)swapchain_info.extent.width, (float)swapchain_info.extent.height, 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, swapchain_info.extent};
    vkCmdSetViewport(swapchain_info.primary_command_buffers[image_index], 0, 1, &viewport);
    vkCmdSetScissor(swapchain_info.primary_command_buffers[image_index], 0, 1, &scissor);
}

void CommandBindVertexBuffer(VkBuffer buffer, size_t *offsets, u32 image_index)
{
    VkBuffer buffers[7] = {buffer, buffer, buffer, buffer, buffer, buffer, buffer};
    vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 7, buffers, offsets);
}

void CommandBindIndexBuffer(VkBuffer buffer, VkIndexType type, u32 image_index)
{
    vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], buffer, 0, type);
}

void CommandPushConstants(VkPipelineLayout pipeline_layout, u32 stages, const PushConstantBlock *push_block, u32 image_index)
{
    vkCmdPushConstants(swapchain_info.primary_command_buffers[image_index], pipeline_layout, stages, 0, sizeof(PushConstantBlock), (void *)push_block);
}

void CommandDrawIndexed(u32 image_index, u32 index_count)
{
    vkCmdDrawIndexed(swapchain_info.primary_command_buffers[image_index], index_count, 1, 0, 0, 0);
}

void CommandEndRenderPass(u32 image_index)
{
    vkCmdEndRenderPass(swapchain_info.primary_command_buffers[image_index]);
    VK_CHECK_RESULT(vkEndCommandBuffer(swapchain_info.primary_command_buffers[image_index]));
}

void GetSwapchainExtent(u32 *width, u32 *height)
{
    *width = swapchain_info.extent.width;
    *height = swapchain_info.extent.height;
}

u32 GetSwapchainImageCount()
{
    return swapchain_info.image_count;
}

VkRenderPass GetSwapchainRenderPass()
{
    return swapchain_info.renderpass;
}

void WaitDeviceIdle()
{
    vkDeviceWaitIdle(vulkan_info.logical_device);
}