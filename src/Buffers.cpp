#include <Buffers.h>

// This is a temporary file

void CreateDataBuffer(uint8_t data_count, size_t size_data_element, const void* data, VkBuffer* buffer,
                   VkDeviceMemory* buffer_memory)
{
    //VkDeviceSize buffer_size = sizeof(Vertex) * model->vertex_count;
    VkDeviceSize buffer_size = size_per_vertex * vertex_count;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void* data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices, (size_t) buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);
    
    // CopyBuffer(staging_buffer, model->vertex_buffer, buffer_size);
    CopyBuffer(staging_buffer, vertex_buffer, buffer_size);

    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void CreateUniformBuffers(const size_t buffer_struct_size, VkBuffer* uniform_buffers, 
                          VkDeviceMemory* uniform_buffers_memory,
                          const SwapchainInfo* swapchain_info)
{
    // VkDeviceSize buffer_size = sizeof(UniformBufferObject);
    VkDeviceSize buffer_size = sizeof(buffer_struct_size);

    uniform_buffers = (VkBuffer *)malloc(sizeof(VkBuffer) * swapchain_info.image_count);
    uniform_buffers_memory = (VkDeviceMemory *)malloc(sizeof(VkDeviceMemory) * swapchain_info.image_count);
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        // Calls function in RenderBase.cpp
        CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers[i], uniform_buffers_memory[i]);
    }
}

// This should definately not be outside of render base
void CreateDescriptorSets(const SwapchainInfo* swapchain_info, const VulkanInfo* vulkan_info,
                     const VkBuffer* uniform_buffers);
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
        descriptor_info.buffer = uniform_buffers[i];
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