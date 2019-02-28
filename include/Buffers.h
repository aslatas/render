
#pragma once

#include "VulkanLoader.h"

// This is a temporary file to prevent myself from modifying RenderBase

// This is named this way because there is a CreateBuffer function in RenderBase and I do not want
// external conflicts.
// This function creates 
// vertex_count: number of Vertices
// size_per_vertex: size of the struct/array containing the vertex data
// vertices: vertx data
// buffer: buffer to load the vertex data into
void CreateDataBuffer(uint8_t data_count, size_t size_data_element, const void* data, VkBuffer* buffer,
                   VkDeviceMemory* buffer_memory);

// vertex_count: number of Vertices
// size_per_vertex: size of the struct/array containing the vertex data
// vertices: vertx data
// buffer: buffer to load the vertex data into
// void CreateIndexBuffer(uint8_t index_count, size_t size_per_index, const void* indices, VkBuffer* index_buffer,
//                   VkDeviceMemory* index_buffer_memory);

// This function creates uniform buffers for a Model in the program.
// buffer_struct_size: size of the struct the buffer data is being loaded into.
// uniform_buffers: an array of size of # of swap chain images.
// uniform_buffer_memory: an array of size of # of swap chain images.
// swapchain_info: swapchain info to load the uniform_buffers into
void CreateUniformBuffers(const size_t buffer_struct_size, VkBuffer* uniform_buffers, 
                          VkDeviceMemory* uniform_buffer_memory,
                          const SwapchainInfo* swapchain_info);

// Create a Descriptor Set for a given Uniform Buffer. Generally will be  assiciated with a model.
// Only 32 descriptor sets are allowed to be active at one time.
// swapchain_info: info for the current swap chain
// vulkan_info: global vulkan state
// uniform_buffers: uniform buffers for the descriptor set
void CreateDescriptorSets(const SwapchainInfo* swapchain_info, const VulkanInfo* vulkan_info,
                     const VkBuffer* uniform_buffers);