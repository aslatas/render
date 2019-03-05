// TODO(Matt): There are a few platform specific bits lingering in here.
#include "RenderBase.h"
#include "VulkanInit.h"
#include "Main.h"
#include "Font.h"

static VulkanInfo vulkan_info = {};
static SwapchainInfo swapchain_info = {};
BitmapFont font;
Texture texture;
static Model *boxes;
glm::vec3 initial_positions[3] = {{-0.3f, -0.3f, -0.3f},{0.3f, 0.3f, -0.3f}, {0.0f, 0.0f, 0.3f}}; 
uint32_t box_count = 3;
uint32_t material_count = 5;
uint32_t selected_boxes[3] = {0, 0, 0};
uint32_t selected_count = 0;

char *ReadShaderFile(const char *path, uint32_t *length)
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

void InitializeRenderer()
{
    InitializeVulkan(&vulkan_info, &swapchain_info);
    boxes = (Model *)malloc(sizeof(Model) * box_count);
    glm::vec3 box_pos = glm::vec3(-0.3f, -0.3f, -0.3f);
    glm::vec3 box_ext = glm::vec3(0.5f, 0.5f, 0.5f);
    boxes[0] = CreateBox(box_pos, box_ext, 0, swapchain_info.image_count);
    box_pos = glm::vec3(0.3f, 0.3f, -0.3f);
    boxes[1] = CreateBox(box_pos, box_ext, 1, swapchain_info.image_count);
    box_pos = glm::vec3(0.0f, 0.0f, 0.3f);
    CreatePipeline(&swapchain_info.pipelines[0], &swapchain_info.pipeline_layouts[0], "shaders/vert.spv", "shaders/frag.spv");
    CreatePipeline(&swapchain_info.pipelines[1], &swapchain_info.pipeline_layouts[1], "shaders/vert2.spv", "shaders/frag2.spv");
    CreateStencilPipeline(&swapchain_info.pipelines[2], &swapchain_info.pipeline_layouts[2],  "shaders/stencil_vert.spv");
    CreateOutlinePipeline(&swapchain_info.pipelines[3], &swapchain_info.pipeline_layouts[3],  "shaders/outline_vert.spv", "shaders/outline_frag.spv");
    CreatePipeline(&swapchain_info.pipelines[4], &swapchain_info.pipeline_layouts[4], "shaders/text_vert.spv", "shaders/text_frag.spv");
    texture = LoadTexture(&vulkan_info, "textures/proto.jpg", 4, true);
    font = LoadBitmapFont(&vulkan_info, "fonts/Hind-Regular.ttf", 4);
    boxes[2] = CreateText("Hello, World!", &font, {25.0f, 128.0f}, {(float)swapchain_info.extent.width, (float)swapchain_info.extent.height});
    // TODO(Matt): Find a different solution for buffer allocation.
    for (uint32_t i = 0; i < box_count; ++i)
    {
        CreateVertexBuffer(&boxes[i]);
        CreateIndexBuffer(&boxes[i]);
        CreateUniformBuffers(&boxes[i]);
        CreateDescriptorSets(&boxes[i]);
    }
}

void ShutdownRenderer()
{
    ShutdownVulkan(&vulkan_info, &swapchain_info);
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
void CreateStencilPipeline(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout, char *vert_file)
{
    uint32_t vert_length;
    char *vert_code = ReadShaderFile(vert_file, &vert_length);
    if (vert_code == nullptr)
    {
        std::cerr << "Unable to read shader files!" << std::endl;
        exit(EXIT_FAILURE);
    }
    VkShaderModule vert_module = CreateShaderModule(vert_code, vert_length);
    
    VkPipelineShaderStageCreateInfo vert_create_info = {};
    vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.module = vert_module;
    vert_create_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo stages[] = {vert_create_info};
    
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
    raster_create_info.cullMode = VK_CULL_MODE_NONE;
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
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_TRUE;
    depth_stencil.back = {};
    depth_stencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depth_stencil.back.failOp = VK_STENCIL_OP_REPLACE;
    depth_stencil.back.depthFailOp = VK_STENCIL_OP_REPLACE;
    depth_stencil.back.passOp = VK_STENCIL_OP_REPLACE;
    depth_stencil.back.compareMask = 0xff;
    depth_stencil.back.writeMask = 0xff;
    depth_stencil.back.reference = 1;
    depth_stencil.front = depth_stencil.back;
    
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &vulkan_info.descriptor_set_layout;
    
    if (vkCreatePipelineLayout(vulkan_info.logical_device, &layout_create_info, nullptr, &swapchain_info.pipeline_layouts[2]) != VK_SUCCESS)
    {
        std::cerr << "Unable to create stencil pipeline layout!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 1;
    pipeline_create_info.pStages = stages;
    pipeline_create_info.pVertexInputState = &input_create_info;
    pipeline_create_info.pInputAssemblyState = &assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_create_info;
    pipeline_create_info.pRasterizationState = &raster_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pColorBlendState = &blend_create_info;
    pipeline_create_info.pDepthStencilState = &depth_stencil;
    pipeline_create_info.layout = *pipeline_layout;
    pipeline_create_info.renderPass = swapchain_info.renderpass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, pipeline) != VK_SUCCESS)
    {
        std::cerr << "Unable to create stencil pipeline!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkDestroyShaderModule(vulkan_info.logical_device, vert_module, nullptr);
}

void CreateOutlinePipeline(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout, char *vert_file, char *frag_file)
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
    raster_create_info.cullMode = VK_CULL_MODE_NONE;
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
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_TRUE;
    depth_stencil.back = {};
    depth_stencil.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
    depth_stencil.back.failOp = VK_STENCIL_OP_KEEP;
    depth_stencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depth_stencil.back.passOp = VK_STENCIL_OP_REPLACE;
    depth_stencil.back.compareMask = 0xff;
    depth_stencil.back.writeMask = 0xff;
    depth_stencil.back.reference = 1;
    depth_stencil.front = depth_stencil.back;
    
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &vulkan_info.descriptor_set_layout;
    
    if (vkCreatePipelineLayout(vulkan_info.logical_device, &layout_create_info, nullptr, pipeline_layout) != VK_SUCCESS)
    {
        std::cerr << "Unable to create outline pipeline layout!" << std::endl;
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
    pipeline_create_info.layout = *pipeline_layout;
    pipeline_create_info.renderPass = swapchain_info.renderpass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, pipeline) != VK_SUCCESS)
    {
        std::cerr << "Unable to create outline pipeline!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    vkDestroyShaderModule(vulkan_info.logical_device, frag_module, nullptr);
    vkDestroyShaderModule(vulkan_info.logical_device, vert_module, nullptr);
}

void CreatePipeline(VkPipeline *pipeline, VkPipelineLayout *pipeline_layout, char *vert_file, char *frag_file)
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
    layout_create_info.pSetLayouts = &vulkan_info.descriptor_set_layout;
    
    if (vkCreatePipelineLayout(vulkan_info.logical_device, &layout_create_info, nullptr, pipeline_layout) != VK_SUCCESS)
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
    pipeline_create_info.layout = *pipeline_layout;
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

void CreateVertexBuffer(Model *model)
{
    VkDeviceSize buffer_size = sizeof(Vertex) * model->vertex_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model->vertices, (size_t)buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model->vertex_buffer, model->vertex_buffer_memory);
    
    CopyBuffer(&vulkan_info, staging_buffer, model->vertex_buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateIndexBuffer(Model *model)
{
    VkDeviceSize buffer_size = sizeof(uint32_t) * model->index_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model->indices, (size_t)buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model->index_buffer, model->index_buffer_memory);
    
    CopyBuffer(&vulkan_info, staging_buffer, model->index_buffer, buffer_size);
    
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
        CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, model->uniform_buffers[i], model->uniform_buffers_memory[i]);
    }
}

void CreateDescriptorSets(Model *model)
{
    model->descriptor_set_layouts = (VkDescriptorSetLayout *)malloc(sizeof(VkDescriptorSetLayout) * swapchain_info.image_count);
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
    {
        model->descriptor_set_layouts[i] = vulkan_info.descriptor_set_layout;
    }
    
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = model->descriptor_set_layouts;
    model->descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * swapchain_info.image_count);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, model->descriptor_sets));
    if (model->shader_id == 4) {
        for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
        {
            VkDescriptorBufferInfo descriptor_info = {};
            descriptor_info.buffer = model->uniform_buffers[i];
            descriptor_info.offset = 0;
            descriptor_info.range = sizeof(UniformBufferObject);
            
            VkDescriptorImageInfo image_info = {};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = font.texture.image_view;
            image_info.sampler = font.texture.sampler;
            
            VkWriteDescriptorSet uniform_write = {};
            uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uniform_write.dstSet = model->descriptor_sets[i];
            uniform_write.dstBinding = 0;
            uniform_write.dstArrayElement = 0;
            uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uniform_write.descriptorCount = 1;
            uniform_write.pBufferInfo = &descriptor_info;
            
            VkWriteDescriptorSet sampler_write = {};
            sampler_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            sampler_write.dstSet = model->descriptor_sets[i];
            sampler_write.dstBinding = 1;
            sampler_write.dstArrayElement = 0;
            sampler_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            sampler_write.descriptorCount = 1;
            sampler_write.pImageInfo = &image_info;
            VkWriteDescriptorSet descriptor_writes[] = {uniform_write, sampler_write};
            vkUpdateDescriptorSets(vulkan_info.logical_device, 2, descriptor_writes, 0, nullptr);
        }
    } else {
        for (uint32_t i = 0; i < swapchain_info.image_count; ++i)
        {
            VkDescriptorBufferInfo descriptor_info = {};
            descriptor_info.buffer = model->uniform_buffers[i];
            descriptor_info.offset = 0;
            descriptor_info.range = sizeof(UniformBufferObject);
            
            VkDescriptorImageInfo image_info = {};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = texture.image_view;
            image_info.sampler = texture.sampler;
            
            VkWriteDescriptorSet uniform_write = {};
            uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uniform_write.dstSet = model->descriptor_sets[i];
            uniform_write.dstBinding = 0;
            uniform_write.dstArrayElement = 0;
            uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uniform_write.descriptorCount = 1;
            uniform_write.pBufferInfo = &descriptor_info;
            
            VkWriteDescriptorSet sampler_write = {};
            sampler_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            sampler_write.dstSet = model->descriptor_sets[i];
            sampler_write.dstBinding = 1;
            sampler_write.dstArrayElement = 0;
            sampler_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            sampler_write.descriptorCount = 1;
            sampler_write.pImageInfo = &image_info;
            VkWriteDescriptorSet descriptor_writes[] = {uniform_write, sampler_write};
            vkUpdateDescriptorSets(vulkan_info.logical_device, 2, descriptor_writes, 0, nullptr);
        }
    }
}

void RecordPrimaryCommand(uint32_t image_index)
{
    VkCommandBufferBeginInfo buffer_begin_info = {};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    VK_CHECK_RESULT(vkBeginCommandBuffer(swapchain_info.primary_command_buffers[image_index], &buffer_begin_info));
    
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
    
    for (uint32_t model_index = 0; model_index < box_count; ++model_index)
    {
        vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipelines[boxes[model_index].shader_id]);
        VkBuffer vertex_buffers[] = {boxes[model_index].vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], boxes[model_index].index_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipeline_layouts[boxes[model_index].shader_id], 0, 1, &boxes[model_index].descriptor_sets[image_index], 0, nullptr);
        vkCmdDrawIndexed(swapchain_info.primary_command_buffers[image_index], boxes[model_index].index_count, 1, 0, 0, 0);
    }
    
    for (uint32_t outline_stage = 2; outline_stage <= 3; ++outline_stage) {
        if (selected_count == 0) {
            break;
        }
        vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipelines[outline_stage]);
        for (uint32_t selected_index = 0; selected_index < selected_count; ++selected_index) {
            VkBuffer vertex_buffers[] = {boxes[selected_boxes[selected_index]].vertex_buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], boxes[selected_boxes[selected_index]].index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain_info.pipeline_layouts[outline_stage], 0, 1, &boxes[selected_boxes[selected_index]].descriptor_sets[image_index], 0, nullptr);
            vkCmdDrawIndexed(swapchain_info.primary_command_buffers[image_index], boxes[selected_boxes[selected_index]].index_count, 1, 0, 0, 0);
        }
    }
    
    vkCmdEndRenderPass(swapchain_info.primary_command_buffers[image_index]);
    
    VK_CHECK_RESULT(vkEndCommandBuffer(swapchain_info.primary_command_buffers[image_index]));
}

void DrawFrame()
{
    vkWaitForFences(vulkan_info.logical_device, 1, &vulkan_info.in_flight_fences[swapchain_info.current_frame], VK_TRUE, 0xffffffffffffffff);
    
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(vulkan_info.logical_device, swapchain_info.swapchain, 0xffffffffffffffff, vulkan_info.image_available_semaphores[swapchain_info.current_frame], VK_NULL_HANDLE, &image_index);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain(&vulkan_info, &swapchain_info);
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
    
    RecordPrimaryCommand(image_index);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore wait_semaphores[] = {vulkan_info.image_available_semaphores[swapchain_info.current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &swapchain_info.primary_command_buffers[image_index];
    VkSemaphore signal_semaphores[] = {vulkan_info.render_finished_semaphores[swapchain_info.current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    vkResetFences(vulkan_info.logical_device, 1, &vulkan_info.in_flight_fences[swapchain_info.current_frame]);
    
    VK_CHECK_RESULT(vkQueueSubmit(vulkan_info.graphics_queue, 1, &submit_info, vulkan_info.in_flight_fences[swapchain_info.current_frame]));
    
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
        RecreateSwapchain(&vulkan_info, &swapchain_info);
    } else if (result != VK_SUCCESS) {
        ExitWithError("Unable to present swapchain image!");
    }
    
    swapchain_info.current_frame = (swapchain_info.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void UpdateModels(double frame_delta)
{
    for (uint32_t i = 0; i < box_count; ++i) {
        boxes[i].rot.z += (float)frame_delta * glm::radians(25.0f);
        boxes[i].ubo.model = glm::translate(glm::mat4(1.0f), initial_positions[i]);
        boxes[i].ubo.model = glm::yawPitchRoll(boxes[i].rot.x, boxes[i].rot.y, boxes[i].rot.z) * boxes[i].ubo.model;
        boxes[i].pos = glm::vec3(boxes[i].ubo.model[3].x, boxes[i].ubo.model[3].y, boxes[i].ubo.model[3].z);
        
        boxes[i].ubo.sun.direction = glm::vec4(0.7f, -0.2f, -1.0f, 0.0f);
        boxes[i].ubo.sun.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
        boxes[i].ubo.sun.specular = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        boxes[i].ubo.sun.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);
    }
    
}

// TODO(Matt): Figure out uniforms in general.
void UpdateUniforms(uint32_t current_image, Model *model)
{
    void *data;
    vkMapMemory(vulkan_info.logical_device, model->uniform_buffers_memory[current_image], 0, sizeof(model->ubo), 0, &data);
    memcpy(data, &model->ubo, sizeof(model->ubo));
    vkUnmapMemory(vulkan_info.logical_device, model->uniform_buffers_memory[current_image]);
}

void SelectObject(int32_t mouse_x, int32_t mouse_y, bool accumulate)
{
    if (!accumulate) selected_count = 0;
    float min_dist = INFINITY;
    int32_t selection = -1;
    for (uint32_t i = 0; i < box_count; ++i) {
        if (!boxes[i].hit_test_enabled) continue;
        float hit_dist;
        Ray ray = ScreenPositionToWorldRay(mouse_x, mouse_y, swapchain_info.extent.width, swapchain_info.extent.height, boxes[i].ubo.view, boxes[i].ubo.projection, 1000.0f);
        glm::vec3 intersection;
        if (RaycastAgainstModelBounds(ray, &boxes[i], &intersection)) {
            hit_dist = glm::distance2(ray.origin, intersection);
            if (hit_dist < min_dist) {
                min_dist = hit_dist;
                selection = i;
            }
        }
    }
    if (selection >= 0) {
        // Dumb hacky section incoming (just to get multiple selct working)
        // Check if this box is already selected.
        bool already_selected = false;
        for (uint32_t j = 0; j < selected_count; ++j) {
            if (selected_boxes[j] == selection) {
                // If so, set the flag and quit checking.
                already_selected = true;
                break;
            }
        }
        // If we are doing multi-select and this box was already selected, deselect it.
        if (already_selected && accumulate) {
            uint32_t new_index = 0;
            // To deselect, iterate through all selected.
            for (uint32_t j = 0; j < selected_count; ++j) {
                // Add back all except the deselected box.
                if (selected_boxes[j] == selection) continue;
                selected_boxes[new_index] = selected_boxes[j];
                new_index++;
            }
            // Decrement selected count.
            selected_count--;
            // End of dumb hacky section.
        } else {
            selected_boxes[selected_count] = selection;
            selected_count++;
        }
    }
}

void OnWindowResized()
{
    RecreateSwapchain(&vulkan_info, &swapchain_info);
}

VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descriptor_layout)
{
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &descriptor_layout;
    
    VK_CHECK_RESULT(vkCreatePipelineLayout(vulkan_info.logical_device, &layout_create_info, nullptr, &layout));
    return layout;
}
PipelineCreateInfo CreatePipelineInfo(const char *vert_file, const char *frag_file, VkPipelineLayout layout, VkRenderPass render_pass, uint32_t subpass)
{
    PipelineCreateInfo result = {};
    
    result.module_count = (frag_file) ? 2 : 1;
    result.shader_modules = (VkShaderModule *)malloc(sizeof(VkShaderModule) * result.module_count);
    VkPipelineShaderStageCreateInfo stages[2];
    uint32_t vert_length = 0;
    char *vert_code = ReadShaderFile(vert_file, &vert_length);
    if (!vert_code) {
        std::cerr << "Failed to read shader file: \"" << vert_file << "\"" << std::endl;
        exit(EXIT_FAILURE);
    }
    result.shader_modules[0] = CreateShaderModule(vert_code, vert_length);
    
    VkPipelineShaderStageCreateInfo vert_create_info = {};
    vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.module = result.shader_modules[0];
    vert_create_info.pName = "main";
    stages[0] = vert_create_info;
    if (frag_file) {
        uint32_t frag_length = 0;
        char *frag_code = ReadShaderFile(frag_file, &frag_length);
        if (!frag_code) {
            std::cerr << "Failed to read shader file: \"" << frag_file << "\"" << std::endl;
            exit(EXIT_FAILURE);
        }
        result.shader_modules[1] = CreateShaderModule(frag_code, frag_length);
        VkPipelineShaderStageCreateInfo frag_create_info = {};
        frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_create_info.module = result.shader_modules[1];
        frag_create_info.pName = "main";
        stages[1] = frag_create_info;
    }
    
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
    
    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = (frag_file) ? 2 : 1;
    pipeline_create_info.pStages = stages;
    pipeline_create_info.pVertexInputState = &input_create_info;
    pipeline_create_info.pInputAssemblyState = &assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_create_info;
    pipeline_create_info.pRasterizationState = &raster_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pColorBlendState = &blend_create_info;
    pipeline_create_info.pDepthStencilState = &depth_stencil;
    pipeline_create_info.layout = layout;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = subpass;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    
    return result;
}

void CreatePipelines(PipelineCreateInfo *create_infos, VkPipeline *pipelines, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i) {
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &create_infos[i].create_info, nullptr, &pipelines[i]));
        for (uint32_t j = 0; j < create_infos[i].module_count; ++j) {
            vkDestroyShaderModule(vulkan_info.logical_device, create_infos[i].shader_modules[j], nullptr);
        }
        free(create_infos[i].shader_modules);
    }
}