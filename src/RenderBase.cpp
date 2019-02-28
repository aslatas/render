// TODO(Matt): There are a few platform specific bits lingering in here.
// Move surface creation and surface size query out.
#include "RenderBase.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "RenderTypes.h"
static VulkanInfo vulkan_info = {};
static SwapchainInfo swapchain_info = {};

static Model *boxes;
uint32_t box_count = 3;
uint32_t material_count = 2;

char *ReadShaderFile(char *path, uint32_t *length)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        return nullptr;
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(*length);
    fread(buffer, 1, *length, file);
    fclose(file);
    return buffer;
}

void InitializeVulkan()
{
    boxes = (Model *)malloc(sizeof(Model) * box_count);
    glm::vec3 box_pos = glm::vec3(-0.3f, -0.3f, -0.3f);
    glm::vec3 box_ext = glm::vec3(0.5f, 0.5f, 0.5f);
    boxes[0] = CreateBox(box_pos, box_ext, 0);
    box_pos = glm::vec3(0.3, 0.3, -0.3);
    boxes[1] = CreateBox(box_pos, box_ext, 1);
    box_pos = glm::vec3(0.0f, 0.0f, 0.3f);
    boxes[2] = CreateBox(box_pos, box_ext, 0);
    
    // TODO(Matt): Platform specific.
    Win32LoadVulkanLibrary();
    LoadVulkanGlobalFunctions();
    CreateInstance();
    LoadVulkanInstanceFunctions(vulkan_info.instance);
    LoadVulkanInstanceExtensionFunctions(vulkan_info.instance);
    if (enable_validation)
        CreateDebugMessenger();
    CreateSurface();
    ChoosePhysicalDevice();
    CreateLogicalDevice();
    LoadVulkanDeviceFunctions(vulkan_info.logical_device);
    LoadVulkanDeviceExtensionFunctions(vulkan_info.logical_device);
    CreateSwapchain();
    CreateImageviews();
    CreateRenderpass();
    CreateDescriptorSetLayout();
    CreatePipeline(&swapchain_info.pipelines[0], "shaders/vert.spv", "shaders/frag.spv");
    CreatePipeline(&swapchain_info.pipelines[1], "shaders/vert2.spv", "shaders/frag2.spv");
    CreateCommandPool();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    CreateTextureImage("textures/proto.jpg");
    CreateTextureImageView();
    CreateTextureSampler();
    CreateDescriptorPool();
    // TODO(Matt): Find a different solution for buffer allocation.
    for (uint32_t i = 0; i < box_count; ++i)
    {
        CreateVertexBuffer(&boxes[i]);
        CreateIndexBuffer(&boxes[i]);
        CreateUniformBuffers(&boxes[i]);
        CreateDescriptorSets(&boxes[i]);
    }
    CreateCommandBuffers();
    CreateSyncPrimitives();
}

void CreateInstance()
{
    // TODO(Matt): Better handling of app/engine name and versions. ini?
    if (enable_validation)
    {
        uint32_t available_count;
        VkLayerProperties *available;
        vkEnumerateInstanceLayerProperties(&available_count, nullptr);
        available = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * available_count);
        vkEnumerateInstanceLayerProperties(&available_count, available);
        if (!CheckValidationLayerSupport(available, available_count))
        {
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
    
    if (enable_validation)
    {
        create_info.enabledLayerCount = validation_layer_count;
        create_info.ppEnabledLayerNames = validation_layers;
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }
    
    if (vkCreateInstance(&create_info, nullptr, &vulkan_info.instance) != VK_SUCCESS)
    {
        std::cerr << "Unable to create Vulkan Instance!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Debug callback relays messages from validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
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
    
    if (vkCreateDebugUtilsMessengerEXT(vulkan_info.instance, &create_info, nullptr, &vulkan_info.debug_messenger) != VK_SUCCESS)
    {
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
    
    if (vkCreateWin32SurfaceKHR(vulkan_info.instance, &create_info, nullptr, &vulkan_info.surface) != VK_SUCCESS)
    {
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
    
    if (available_count == 0)
    {
        std::cerr << "No Vulkan-supported devices found!" << std::endl;
        exit(EXIT_FAILURE);
    }
    available_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * available_count);
    vkEnumeratePhysicalDevices(vulkan_info.instance, &available_count, available_devices);
    // Scan the list for suitable devices.
    for (uint32_t i = 0; i < available_count; ++i)
    {
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
        for (uint32_t i = 0; i < family_count; ++i)
        {
            if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphics_index = i;
                graphics_found = true;
            }
            VkBool32 present_supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan_info.surface, &present_supported);
            if (queue_families[i].queueCount > 0 && present_supported)
            {
                present_index = i;
                present_found = true;
            }
        }
        
        free(queue_families);
        if (graphics_found && present_found)
        {
            use_shared_queue = (graphics_index == present_index);
            queues_supported = true;
        }
        else
        {
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
        if (!extensions_supported)
            continue;
        
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
        if (!swapchain_supported)
            continue;
        
        // TODO(Matt): Move required device features (like anisotropic filtering)
        // somewhere else. Maybe just don't use it if unsupported?
        VkPhysicalDeviceFeatures supported_features;
        vkGetPhysicalDeviceFeatures(device, &supported_features);
        if (queues_supported && extensions_supported && swapchain_supported && supported_features.samplerAnisotropy && supported_features.sampleRateShading)
        {
            vulkan_info.physical_device = device;
            vulkan_info.msaa_samples = GetMSAASampleCount();
            vulkan_info.graphics_index = graphics_index;
            vulkan_info.present_index = present_index;
            vulkan_info.use_shared_queue = use_shared_queue;
            break;
        }
    }
    
    free(available_devices);
    if (vulkan_info.physical_device == VK_NULL_HANDLE)
    {
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
    for (uint32_t i = 0; i < family_count; ++i)
    {
        create_infos[i] = {};
        create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_infos[i].queueFamilyIndex = (i == 0) ? vulkan_info.graphics_index : vulkan_info.present_index;
        create_infos[i].queueCount = 1;
        create_infos[i].pQueuePriorities = &queue_priority;
    }
    
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
    if (enable_validation)
    {
        create_info.enabledLayerCount = validation_layer_count;
        create_info.ppEnabledLayerNames = validation_layers;
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(vulkan_info.physical_device, &create_info, nullptr, &vulkan_info.logical_device) != VK_SUCCESS)
    {
        std::cout << "Unable to create logical device!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkGetDeviceQueue(vulkan_info.logical_device, vulkan_info.graphics_index, 0, &vulkan_info.graphics_queue);
    if (!vulkan_info.use_shared_queue)
    {
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
    
    if (available_count == 1 && available[0].format == VK_FORMAT_UNDEFINED)
    {
        swapchain_info.format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        free(available);
        return;
    }
    
    for (uint32_t i = 0; i < available_count; ++i)
    {
        if (available[i].format == VK_FORMAT_B8G8R8A8_UNORM && available[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
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
    for (uint32_t i = 0; i < available_count; ++i)
    {
        if (available[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
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
    if (capabilities.maxImageCount > 0 && swapchain_info.image_count > capabilities.maxImageCount)
    {
        swapchain_info.image_count = capabilities.maxImageCount;
    }
    if (capabilities.currentExtent.width != 0xffffffff)
    {
        swapchain_info.extent = capabilities.currentExtent;
    }
    else
    {
        // TODO(Matt): Platform specific.
        uint32_t width, height;
        Win32GetSurfaceSize(&width, &height);
        swapchain_info.extent = {width, height};
    }
    if (swapchain_info.extent.width > capabilities.maxImageExtent.width)
    {
        swapchain_info.extent.width = capabilities.maxImageExtent.width;
    }
    if (swapchain_info.extent.width < capabilities.minImageExtent.width)
    {
        swapchain_info.extent.width = capabilities.minImageExtent.width;
    }
    if (swapchain_info.extent.height > capabilities.maxImageExtent.height)
    {
        swapchain_info.extent.height = capabilities.maxImageExtent.height;
    }
    if (swapchain_info.extent.height < capabilities.minImageExtent.height)
    {
        swapchain_info.extent.height = capabilities.minImageExtent.height;
    }
}

// TODO(Matt): Support swapchain creation while the old one still exists.
// Defer destruction of the old one to speed it up.
void CreateSwapchain()
{
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseSwapchainExtent();
    swapchain_info.pipelines = (VkPipeline *)malloc(sizeof(VkPipeline) * material_count);
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vulkan_info.surface;
    
    create_info.minImageCount = swapchain_info.image_count;
    create_info.imageFormat = swapchain_info.format.format;
    create_info.imageColorSpace = swapchain_info.format.colorSpace;
    create_info.imageExtent = swapchain_info.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (vulkan_info.use_shared_queue)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        uint32_t queue_indices[] = {vulkan_info.graphics_index, vulkan_info.present_index};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_indices;
    }
    
    create_info.preTransform = swapchain_info.transform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = swapchain_info.present_mode;
    create_info.clipped = VK_TRUE;
    
    if (vkCreateSwapchainKHR(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.swapchain) != VK_SUCCESS)
    {
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
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        swapchain_info.imageviews[i] = CreateImageView(swapchain_info.images[i], swapchain_info.format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void CreateRenderpass()
{
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
    
    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = FindDepthFormat();
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
    
    VkRenderPassCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 3;
    create_info.pAttachments = attachments;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;
    
    if (vkCreateRenderPass(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.renderpass) != VK_SUCCESS)
    {
        std::cerr << "Unable to create renderpass!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uniform_binding = {};
    uniform_binding.binding = 0;
    uniform_binding.descriptorCount = 1;
    uniform_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_binding.pImmutableSamplers = nullptr;
    uniform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutBinding sampler_binding = {};
    sampler_binding.binding = 1;
    sampler_binding.descriptorCount = 1;
    sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_binding.pImmutableSamplers = nullptr;
    sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutBinding bindings[] = {uniform_binding, sampler_binding};
    VkDescriptorSetLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 2;
    create_info.pBindings = bindings;
    
    if (vkCreateDescriptorSetLayout(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.descriptor_set_layout) != VK_SUCCESS)
    {
        std::cerr << "Unable to create descriptor set layout!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

static VkShaderModule CreateShaderModule(char *code, uint32_t length)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = length;
    create_info.pCode = reinterpret_cast<uint32_t *>(code);
    
    VkShaderModule module;
    if (vkCreateShaderModule(vulkan_info.logical_device, &create_info, nullptr, &module) != VK_SUCCESS)
    {
        std::cerr << "Unable to create shader module!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return module;
}

// TODO(Matt): Rework shader loading/swapping once geometry is sorted.
void CreatePipeline(VkPipeline *pipeline, char *vert_file, char *frag_file)
{
    uint32_t vert_length;
    uint32_t frag_length;
    char *vert_code = ReadShaderFile(vert_file, &vert_length);
    char *frag_code = ReadShaderFile(frag_file, &frag_length);
    if (vert_code == nullptr || frag_code == nullptr)
    {
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
    VkVertexInputBindingDescription binding_description = {};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    VkVertexInputAttributeDescription attribute_descriptions[6];
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = 0;
    
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = 12;
    
    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attribute_descriptions[2].offset = 24;
    
    attribute_descriptions[3].binding = 0;
    attribute_descriptions[3].location = 3;
    attribute_descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[3].offset = 40;
    
    attribute_descriptions[4].binding = 0;
    attribute_descriptions[4].location = 4;
    attribute_descriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[4].offset = 48;
    
    attribute_descriptions[5].binding = 0;
    attribute_descriptions[5].location = 5;
    attribute_descriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[5].offset = 56;
    
    input_create_info.vertexBindingDescriptionCount = 1;
    input_create_info.vertexAttributeDescriptionCount = 6;
    input_create_info.pVertexBindingDescriptions = &binding_description;
    input_create_info.pVertexAttributeDescriptions = attribute_descriptions;
    
    VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
    assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly_create_info.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchain_info.extent.width;
    viewport.height = (float)swapchain_info.extent.height;
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
    multisample_create_info.sampleShadingEnable = VK_TRUE;
    multisample_create_info.rasterizationSamples = vulkan_info.msaa_samples;
    
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
    
    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};
    
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &swapchain_info.descriptor_set_layout;
    
    if (vkCreatePipelineLayout(vulkan_info.logical_device, &layout_create_info, nullptr, &swapchain_info.pipeline_layout) != VK_SUCCESS)
    {
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
    pipeline_create_info.pDepthStencilState = &depth_stencil;
    pipeline_create_info.layout = swapchain_info.pipeline_layout;
    pipeline_create_info.renderPass = swapchain_info.renderpass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, pipeline) != VK_SUCCESS)
    {
        std::cerr << "Unable to create pipeline!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkDestroyShaderModule(vulkan_info.logical_device, frag_module, nullptr);
    vkDestroyShaderModule(vulkan_info.logical_device, vert_module, nullptr);
}

void CreateFramebuffers()
{
    swapchain_info.framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchain_info.image_count);
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
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
        
        if (vkCreateFramebuffer(vulkan_info.logical_device, &create_info, nullptr, &swapchain_info.framebuffers[i]) != VK_SUCCESS)
        {
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
    
    if (vkCreateCommandPool(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.command_pool) != VK_SUCCESS)
    {
        std::cerr << "Unable to create command pool!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateVertexBuffer(Model *model)
{
    VkDeviceSize buffer_size = sizeof(Vertex) * model->vertex_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model->vertices, (size_t)buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model->vertex_buffer, model->vertex_buffer_memory);
    
    CopyBuffer(staging_buffer, model->vertex_buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateIndexBuffer(Model *model)
{
    VkDeviceSize buffer_size = sizeof(uint32_t) * model->index_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model->indices, (size_t)buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model->index_buffer, model->index_buffer_memory);
    
    CopyBuffer(staging_buffer, model->index_buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateUniformBuffers(Model *model)
{
    VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    model->uniform_buffers = (VkBuffer *)malloc(sizeof(VkBuffer) * swapchain_info.image_count);
    model->uniform_buffers_memory = (VkDeviceMemory *)malloc(sizeof(VkDeviceMemory) * swapchain_info.image_count);
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, model->uniform_buffers[i], model->uniform_buffers_memory[i]);
    }
}

void CreateDescriptorPool()
{
    VkDescriptorPoolSize uniform_size = {};
    uniform_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_size.descriptorCount = swapchain_info.image_count * box_count;
    
    VkDescriptorPoolSize sampler_size = {};
    sampler_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_size.descriptorCount = swapchain_info.image_count * box_count;
    
    VkDescriptorPoolSize pool_sizes[] = {uniform_size, sampler_size};
    VkDescriptorPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 2;
    create_info.pPoolSizes = pool_sizes;
    create_info.maxSets = swapchain_info.image_count * box_count;
    
    if (vkCreateDescriptorPool(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.descriptor_pool) != VK_SUCCESS)
    {
        std::cerr << "Unable to create descriptor pool!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateDescriptorSets(Model *model)
{
    swapchain_info.descriptor_set_layouts = (VkDescriptorSetLayout *)malloc(sizeof(VkDescriptorSetLayout) * swapchain_info.image_count);
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        swapchain_info.descriptor_set_layouts[i] = swapchain_info.descriptor_set_layout;
    }
    
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = swapchain_info.descriptor_set_layouts;
    swapchain_info.descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * swapchain_info.image_count);
    if (vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, swapchain_info.descriptor_sets) != VK_SUCCESS)
    {
        std::cerr << "Unable to create descriptor sets!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        VkDescriptorBufferInfo descriptor_info = {};
        descriptor_info.buffer = model->uniform_buffers[i];
        descriptor_info.offset = 0;
        descriptor_info.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = vulkan_info.texture_image_view;
        image_info.sampler = vulkan_info.texture_sampler;
        
        VkWriteDescriptorSet uniform_write = {};
        uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniform_write.dstSet = swapchain_info.descriptor_sets[i];
        uniform_write.dstBinding = 0;
        uniform_write.dstArrayElement = 0;
        uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniform_write.descriptorCount = 1;
        uniform_write.pBufferInfo = &descriptor_info;
        
        VkWriteDescriptorSet sampler_write = {};
        sampler_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        sampler_write.dstSet = swapchain_info.descriptor_sets[i];
        sampler_write.dstBinding = 1;
        sampler_write.dstArrayElement = 0;
        sampler_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_write.descriptorCount = 1;
        sampler_write.pImageInfo = &image_info;
        VkWriteDescriptorSet descriptor_writes[] = {uniform_write, sampler_write};
        vkUpdateDescriptorSets(vulkan_info.logical_device, 2, descriptor_writes, 0, nullptr);
    }
}

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(vulkan_info.logical_device, &create_info, nullptr, &buffer) != VK_SUCCESS)
    {
        std::cerr << "Unable to create buffer!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(vulkan_info.logical_device, buffer, &requirements);
    
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(vulkan_info.logical_device, &allocate_info, nullptr, &memory) != VK_SUCCESS)
    {
        std::cerr << "Unable to allocate buffer memory!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkBindBufferMemory(vulkan_info.logical_device, buffer, memory, 0);
}

void CopyBuffer(VkBuffer source, VkBuffer destination, VkDeviceSize size)
{
    VkCommandBuffer command_buffer = BeginOneTimeCommand();
    
    VkBufferCopy copy_region = {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);
    EndOneTimeCommand(command_buffer);
}

uint32_t FindMemoryType(uint32_t type, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties available_properties;
    vkGetPhysicalDeviceMemoryProperties(vulkan_info.physical_device, &available_properties);
    
    for (uint32_t i = 0; i < available_properties.memoryTypeCount; ++i)
    {
        if ((type & (1 << i)) && (available_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
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
    
    if (vkAllocateCommandBuffers(vulkan_info.logical_device, &allocate_info, swapchain_info.command_buffers) != VK_SUCCESS)
    {
        std::cerr << "Unable to allocate command buffers!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        VkCommandBufferBeginInfo buffer_begin_info = {};
        buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        
        if (vkBeginCommandBuffer(swapchain_info.command_buffers[i], &buffer_begin_info) != VK_SUCCESS)
        {
            std::cerr << "Unable to begin command buffer recording!" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        VkRenderPassBeginInfo pass_begin_info = {};
        pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        pass_begin_info.renderPass = swapchain_info.renderpass;
        pass_begin_info.framebuffer = swapchain_info.framebuffers[i];
        pass_begin_info.renderArea.offset = {0, 0};
        pass_begin_info.renderArea.extent = swapchain_info.extent;
        
        VkClearValue clear_colors[2];
        clear_colors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_colors[1].depthStencil = {1.0f, 0};
        pass_begin_info.clearValueCount = 2;
        pass_begin_info.pClearValues = clear_colors;
        
        vkCmdBeginRenderPass(swapchain_info.command_buffers[i], &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        
        for (uint32_t model_index = 0; model_index < box_count; ++model_index)
        {
            
            vkCmdBindPipeline(swapchain_info.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipelines[boxes[model_index].shader_id]);
            VkBuffer vertex_buffers[] = {boxes[model_index].vertex_buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(swapchain_info.command_buffers[i], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(swapchain_info.command_buffers[i], boxes[model_index].index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(swapchain_info.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipeline_layout, 0, 1, &swapchain_info.descriptor_sets[i], 0, nullptr);
            vkCmdDrawIndexed(swapchain_info.command_buffers[i], boxes[model_index].index_count, 1, 0, 0, 0);
        }
        
        vkCmdEndRenderPass(swapchain_info.command_buffers[i]);
        
        if (vkEndCommandBuffer(swapchain_info.command_buffers[i]) != VK_SUCCESS)
        {
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
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(vulkan_info.logical_device, &semaphore_create_info, nullptr, &swapchain_info.image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkan_info.logical_device, &semaphore_create_info, nullptr, &swapchain_info.render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vulkan_info.logical_device, &fence_create_info, nullptr, &swapchain_info.in_flight_fences[i]) != VK_SUCCESS)
        {
            std::cerr << "Unable to create synchronization objects!" << std::endl;
        }
    }
}

void DrawFrame()
{
    vkWaitForFences(vulkan_info.logical_device, 1, &swapchain_info.in_flight_fences[swapchain_info.current_frame], VK_TRUE, 0xffffffffffffffff);
    
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(vulkan_info.logical_device, swapchain_info.swapchain, 0xffffffffffffffff, swapchain_info.image_available_semaphores[swapchain_info.current_frame], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        std::cerr << "Failed to acquire swapchain image!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    for (uint32_t i = 0; i < box_count; ++i)
    {
        UpdateUniforms(image_index, &boxes[i]);
    }
    
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
    
    if (vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, swapchain_info.in_flight_fences[swapchain_info.current_frame]) != VK_SUCCESS)
    {
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
    if (vulkan_info.use_shared_queue)
    {
        result = vkQueuePresentKHR(vulkan_info.graphics_queue, &present_info);
    }
    else
    {
        
        result = vkQueuePresentKHR(vulkan_info.present_queue, &present_info);
    }
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        std::cerr << "Unable to present swapchain image!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    swapchain_info.current_frame = (swapchain_info.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RecreateSwapchain()
{
    vkDeviceWaitIdle(vulkan_info.logical_device);
    CleanupSwapchain();
    ChooseSwapchainExtent();
    while (swapchain_info.extent.width == 0 || swapchain_info.extent.height == 0) {
        if (!Win32PollEvents()) exit(0);
        ChooseSwapchainExtent();
    }
    CreateSwapchain();
    CreateImageviews();
    CreateRenderpass();
    CreatePipeline(&swapchain_info.pipelines[0], "shaders/vert.spv", "shaders/frag.spv");
    CreatePipeline(&swapchain_info.pipelines[1], "shaders/vert2.spv", "shaders/frag2.spv");
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandBuffers();
}

void CleanupSwapchain()
{
    vkDestroyImageView(vulkan_info.logical_device, swapchain_info.color_image_view, nullptr);
    vkDestroyImage(vulkan_info.logical_device, swapchain_info.color_image, nullptr);
    vkFreeMemory(vulkan_info.logical_device, swapchain_info.color_image_memory, nullptr);
    vkDestroyImageView(vulkan_info.logical_device, swapchain_info.depth_image_view, nullptr);
    vkDestroyImage(vulkan_info.logical_device, swapchain_info.depth_image, nullptr);
    vkFreeMemory(vulkan_info.logical_device, swapchain_info.depth_image_memory, nullptr);
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        vkDestroyFramebuffer(vulkan_info.logical_device, swapchain_info.framebuffers[i], nullptr);
    }
    free(swapchain_info.framebuffers);
    
    vkFreeCommandBuffers(vulkan_info.logical_device, vulkan_info.command_pool, swapchain_info.image_count, swapchain_info.command_buffers);
    free(swapchain_info.command_buffers);
    for (uint32_t i = 0; i < swapchain_info.pipeline_count; ++i)
    {
        vkDestroyPipeline(vulkan_info.logical_device, swapchain_info.pipelines[i], nullptr);
    }
    free(swapchain_info.pipelines);
    vkDestroyPipelineLayout(vulkan_info.logical_device, swapchain_info.pipeline_layout, nullptr);
    vkDestroyRenderPass(vulkan_info.logical_device, swapchain_info.renderpass, nullptr);
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        vkDestroyImageView(vulkan_info.logical_device, swapchain_info.imageviews[i], nullptr);
    }
    free(swapchain_info.imageviews);
    
    vkDestroySwapchainKHR(vulkan_info.logical_device, swapchain_info.swapchain, nullptr);
    free(swapchain_info.images);
}

void ShutdownVulkan()
{
    // TODO(Matt): IMPORTANT: actually free memory for pipelines, descriptoer pools, etc.
    // OS is doing a lot.
    vkDeviceWaitIdle(vulkan_info.logical_device);
    CleanupSwapchain();
    vkDestroySampler(vulkan_info.logical_device, vulkan_info.texture_sampler, nullptr);
    vkDestroyImageView(vulkan_info.logical_device, vulkan_info.texture_image_view, nullptr);
    vkDestroyImage(vulkan_info.logical_device, vulkan_info.texture_image, nullptr);
    vkFreeMemory(vulkan_info.logical_device, vulkan_info.texture_memory, nullptr);
    vkDestroyDescriptorPool(vulkan_info.logical_device, vulkan_info.descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(vulkan_info.logical_device, swapchain_info.descriptor_set_layout, nullptr);
    for (uint32_t i = 0; i < box_count; ++i)
    {
        for (uint32_t j = 0; j < swapchain_info.image_count; ++j)
        {
            vkDestroyBuffer(vulkan_info.logical_device, boxes[i].uniform_buffers[j], nullptr);
            vkFreeMemory(vulkan_info.logical_device, boxes[i].uniform_buffers_memory[j], nullptr);
        }
    }
    
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        vkDestroyBuffer(vulkan_info.logical_device, boxes[i].vertex_buffer, nullptr);
        vkFreeMemory(vulkan_info.logical_device, boxes[i].vertex_buffer_memory, nullptr);
        vkDestroyBuffer(vulkan_info.logical_device, boxes[i].index_buffer, nullptr);
        vkFreeMemory(vulkan_info.logical_device, boxes[i].index_buffer_memory, nullptr);
        DestroyModel(&boxes[i]);
    }
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(vulkan_info.logical_device, swapchain_info.render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(vulkan_info.logical_device, swapchain_info.image_available_semaphores[i], nullptr);
        vkDestroyFence(vulkan_info.logical_device, swapchain_info.in_flight_fences[i], nullptr);
    }
    free(swapchain_info.image_available_semaphores);
    free(swapchain_info.render_finished_semaphores);
    free(swapchain_info.in_flight_fences);
    
    vkDestroyCommandPool(vulkan_info.logical_device, vulkan_info.command_pool, nullptr);
    
    vkDestroyDevice(vulkan_info.logical_device, nullptr);
#ifndef NDEBUG
    if (enable_validation)
    {
        vkDestroyDebugUtilsMessengerEXT(vulkan_info.instance, vulkan_info.debug_messenger, nullptr);
    }
#endif
    vkDestroySurfaceKHR(vulkan_info.instance, vulkan_info.surface, nullptr);
    vkDestroyInstance(vulkan_info.instance, nullptr);
    // TODO(Matt): Platform specific.
    Win32FreeVulkanLibrary();
}

// TODO(Matt): Figure out uniforms in general.
void UpdateUniforms(uint32_t current_image, Model *model)
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    
    auto current_time = std::chrono::high_resolution_clock::now();
    float run_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    
    UniformBufferObject ubo = {};
    ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    ubo.model = glm::rotate(glm::mat4(1.0f), run_time * glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(ubo.view_position.x, ubo.view_position.y, ubo.view_position.z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(glm::radians(45.0f), swapchain_info.extent.width / (float)swapchain_info.extent.height, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1;
    
    ubo.sun.direction = glm::vec4(0.7f, -0.2f, -1.0f, 0.0f);
    ubo.sun.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
    ubo.sun.specular = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    ubo.sun.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, model->uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vulkan_info.logical_device, model->uniform_buffers_memory[current_image]);
}

void CreateTextureImage(char *file)
{
    int width, height, channel_count;
    stbi_uc *pixels = stbi_load(file, &width, &height, &channel_count, STBI_rgb_alpha);
    VkDeviceSize image_size = width * height * 4;
    vulkan_info.texture_mips = 1 + (uint32_t)log2(fmax(width, height));
    if (!pixels)
    {
        std::cerr << "Unable to load image file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, image_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    stbi_image_free(pixels);
    CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vulkan_info.texture_image, &vulkan_info.texture_memory, vulkan_info.texture_mips, VK_SAMPLE_COUNT_1_BIT);
    
    TransitionImageLayout(vulkan_info.texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkan_info.texture_mips);
    CopyBufferToImage(staging_buffer, vulkan_info.texture_image, (uint32_t)width, (uint32_t)height);
    
    GenerateMipmaps(vulkan_info.texture_image, VK_FORMAT_R8G8B8A8_UNORM, width, height, vulkan_info.texture_mips);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory, uint32_t mips, VkSampleCountFlagBits samples)
{
    VkImageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.extent.depth = 1;
    create_info.mipLevels = mips;
    create_info.arrayLayers = 1;
    create_info.format = format;
    create_info.tiling = tiling;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = usage;
    create_info.samples = samples;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateImage(vulkan_info.logical_device, &create_info, nullptr, image) != VK_SUCCESS)
    {
        std::cerr << "Unable to create texture image!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(vulkan_info.logical_device, *image, &requirements);
    
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(vulkan_info.logical_device, &alloc_info, nullptr, image_memory) != VK_SUCCESS)
    {
        std::cerr << "Unable to allocate image memory!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkBindImageMemory(vulkan_info.logical_device, *image, *image_memory, 0);
}

VkCommandBuffer BeginOneTimeCommand()
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = vulkan_info.command_pool;
    alloc_info.commandBufferCount = 1;
    
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vulkan_info.logical_device, &alloc_info, &command_buffer);
    
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(command_buffer, &begin_info);
    
    return command_buffer;
}

void EndOneTimeCommand(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    
    vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan_info.graphics_queue);
    
    vkFreeCommandBuffers(vulkan_info.logical_device, vulkan_info.command_pool, 1, &command_buffer);
}

void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mips)
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
    barrier.subresourceRange.levelCount = mips;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
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
        std::cerr << "Unsupported layout transition!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkCmdPipelineBarrier(command_buffer, source_stage, dest_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    EndOneTimeCommand(command_buffer);
}

void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
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
    
    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndOneTimeCommand(command_buffer);
}

void CreateTextureImageView()
{
    vulkan_info.texture_image_view = CreateImageView(vulkan_info.texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, vulkan_info.texture_mips);
}

VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask, uint32_t mips)
{
    VkImageViewCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.subresourceRange.aspectMask = aspect_mask;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = mips;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    VkImageView image_view;
    if (vkCreateImageView(vulkan_info.logical_device, &create_info, nullptr, &image_view) != VK_SUCCESS)
    {
        std::cerr << "Unable to create image view!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return image_view;
}

void CreateTextureSampler()
{
    VkSamplerCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = VK_FILTER_LINEAR;
    create_info.minFilter = VK_FILTER_LINEAR;
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.anisotropyEnable = VK_TRUE;
    create_info.maxAnisotropy = 16;
    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    create_info.unnormalizedCoordinates = VK_FALSE;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.0f;
    create_info.minLod = 0.0;
    create_info.maxLod = (float)vulkan_info.texture_mips;
    
    if (vkCreateSampler(vulkan_info.logical_device, &create_info, nullptr, &vulkan_info.texture_sampler) != VK_SUCCESS)
    {
        std::cerr << "Unable to create texture sampler!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

VkFormat FindSupportedFormat(VkFormat *acceptable_formats, uint32_t acceptable_count, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    VkFormatProperties properties;
    for (uint32_t i = 0; i < acceptable_count; ++i) {
        vkGetPhysicalDeviceFormatProperties(vulkan_info.physical_device, acceptable_formats[i], &properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) return acceptable_formats[i];
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) return acceptable_formats[i];
    }
    std::cerr << "No acceptable format candidates found!" << std::endl;
    exit(EXIT_FAILURE);
}

VkFormat FindDepthFormat()
{
    VkFormat acceptable_formats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return FindSupportedFormat(acceptable_formats, 2, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void CreateDepthResources()
{
    VkFormat depth_format = FindDepthFormat();
    
    CreateImage(swapchain_info.extent.width, swapchain_info.extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &swapchain_info.depth_image, &swapchain_info.depth_image_memory, 1, vulkan_info.msaa_samples);
    swapchain_info.depth_image_view = CreateImageView(swapchain_info.depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void GenerateMipmaps(VkImage image, VkFormat format,uint32_t width, uint32_t height, uint32_t mips)
{
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(vulkan_info.physical_device, format, &properties);
    if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        std::cerr << "Texture filtering is unsupported!" << std::endl;
        exit(EXIT_FAILURE);
    }
    VkCommandBuffer command_buffer = BeginOneTimeCommand();
    
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    uint32_t mip_width = width;
    uint32_t mip_height = height;
    
    for (uint32_t i = 1; i < mips; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { (int32_t)mip_width, (int32_t)mip_height, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { (mip_width > 1) ? (int32_t)mip_width / 2 : 1, (mip_height > 1) ? (int32_t)mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(command_buffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        
        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = mips - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
    
    EndOneTimeCommand(command_buffer);
}

VkSampleCountFlagBits GetMSAASampleCount()
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(vulkan_info.physical_device, &properties);
    
    VkSampleCountFlags counts = (uint32_t)fmin((float)properties.limits.framebufferColorSampleCounts,(float) properties.limits.framebufferDepthSampleCounts);
    if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
    
    return VK_SAMPLE_COUNT_1_BIT;
}

void CreateColorResources()
{
    VkFormat format = swapchain_info.format.format;
    
    CreateImage(swapchain_info.extent.width, swapchain_info.extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &swapchain_info.color_image, &swapchain_info.color_image_memory, 1, vulkan_info.msaa_samples);
    swapchain_info.color_image_view = CreateImageView(swapchain_info.color_image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    
    TransitionImageLayout(swapchain_info.color_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
}