// TODO(Matt): There are a few platform specific bits lingering in here.
// TODO(Matt): Use a texture list, and have models which use textures
// specify which textures belong in which samplers.
// TODO(Matt): Re-handle the way that uniforms are updated. It's hacky.

#include "RenderBase.h"
#include "VulkanInit.h"
#include "Main.h"
#include "Font.h"

static VulkanInfo vulkan_info = {};
static SwapchainInfo swapchain_info = {};
BitmapFont font;
Texture texture;
MaterialLayout *material_types;

//static Model *boxes;
glm::vec3 initial_positions[3] = {{-0.3f, -0.3f, -0.3f},{0.3f, 0.3f, -0.3f}, {0.0f, 0.0f, 0.3f}}; 
//uint32_t box_count = 3;
//uint32_t selected_boxes[3] = {0, 0, 0};
//uint32_t selected_count = 0;
Model **selected_models = nullptr;
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
    InitializeSceneResources();
    InitializeScene();
}

void ShutdownRenderer()
{
    DestroySwapchain(&vulkan_info, &swapchain_info);
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
    model->uniform_buffers = (VkBuffer *)malloc(sizeof(VkBuffer) * model->uniform_count);
    model->uniform_buffers_memory = (VkDeviceMemory *)malloc(sizeof(VkDeviceMemory) * model->uniform_count);
    for (uint32_t i = 0; i < model->uniform_count; ++i)
    {
        CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, model->uniform_buffers[i], model->uniform_buffers_memory[i]);
    }
}

void CreateDescriptorSets(Model *model)
{
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = material_types[model->material_type].descriptor_layouts;
    model->descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * model->uniform_count);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, model->descriptor_sets));
    
    for (uint32_t i = 0; i < model->uniform_count; ++i)
    {
        VkDescriptorBufferInfo descriptor_info = {};
        descriptor_info.buffer = model->uniform_buffers[i];
        descriptor_info.offset = 0;
        descriptor_info.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // TODO(Matt): Temporary hack to get a second texture sampler for text.
        if (model->shader_id == font.shader_id && model->material_type == font.material_type) {
            
            image_info.imageView = font.texture.image_view;
            image_info.sampler = font.texture.sampler;
            
        } else {
            image_info.imageView = texture.image_view;
            image_info.sampler = texture.sampler;
        }
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

void RecordPrimaryCommand(uint32_t image_index)
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
    
    
    // For each material type.
    for (uint32_t i = 0; i < arrlen(material_types); ++i) {
        MaterialLayout *material_type = &material_types[i];
        // For each material of a given type.
        for (uint32_t j = 0; j < arrlen(material_type->materials); ++j) {
            Material *material = &material_type->materials[j];
            
            // Bind the material pipeline.
            vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
            
            // For each model of a given material.
            for (uint32_t k = 0; k < arrlen(material->models); ++k) {
                Model *model = &material->models[k];
                
                // Bind the vertex, index, and uniform buffers.
                VkBuffer vertex_buffers[] = {model->vertex_buffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 1, vertex_buffers, offsets);
                vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], model->index_buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material_type->pipeline_layout, 0, 1, &model->descriptor_sets[image_index], 0, nullptr);
                
                // Draw the model.
                vkCmdDrawIndexed(swapchain_info.primary_command_buffers[image_index], model->index_count, 1, 0, 0, 0);
            }
        }
    }
    
    // Do post process for outlines.
    // NOTE(Matt): Outlines are done in two passes - one to draw selected
    // into stencil buffer, and one to read stencil buffer for outlines.
    for (uint32_t outline_stage = 2; outline_stage <= 3; ++outline_stage) {
        if (arrlen(selected_models) == 0) {
            break;
        }
        
        // TODO(Matt): Currently post process stuff is hard-coded. Should
        // maybe use a separate set of materials entirely for post process.
        // Bind the stenciling pipeline.
        vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material_types[0].materials[outline_stage].pipeline);
        // For each selected model.
        for (uint32_t i = 0; i < arrlen(selected_models); ++i) {
            // Bind vertex and index buffers, and uniforms.
            VkBuffer vertex_buffers[] = {selected_models[i]->vertex_buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], selected_models[i]->index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material_types[0].pipeline_layout, 0, 1, &selected_models[i]->descriptor_sets[image_index], 0, nullptr);
            // Draw selected models.
            vkCmdDrawIndexed(swapchain_info.primary_command_buffers[image_index], selected_models[i]->index_count, 1, 0, 0, 0);
        }
    }
    
    // End render pass and command buffer recording.
    vkCmdEndRenderPass(swapchain_info.primary_command_buffers[image_index]);
    VK_CHECK_RESULT(vkEndCommandBuffer(swapchain_info.primary_command_buffers[image_index]));
}

void DrawFrame()
{
    // Wait for an image to become available.
    vkWaitForFences(vulkan_info.logical_device, 1, &vulkan_info.in_flight_fences[swapchain_info.current_frame], VK_TRUE, 0xffffffffffffffff);
    
    // Get the next available image.
    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(vulkan_info.logical_device, swapchain_info.swapchain, 0xffffffffffffffff, vulkan_info.image_available_semaphores[swapchain_info.current_frame], VK_NULL_HANDLE, &image_index);
    
    // If out of date, recreate swapchain. Will likely cause a frame hitch.
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain(&vulkan_info, &swapchain_info);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cerr << "Failed to acquire swapchain image!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // Update uniforms to reflect new state of all scene objects.
    // TODO(Matt): Probably only update dynamic object uniforms - static
    // geometry won't change.
    for (uint32_t i = 0; i < arrlen(material_types); ++i) {
        MaterialLayout material_type = material_types[i];
        for (uint32_t j = 0; j < arrlen(material_type.materials); ++j) {
            Material material = material_type.materials[j];
            for (uint32_t k = 0; k < arrlen(material.models); ++k) {
                UpdateUniforms(image_index, &material.models[k]);
            }
        }
    }
    
    // Record draw calls and other work for this frame.
    RecordPrimaryCommand(image_index);
    
    // Submit work for this frame.
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
    
    // Present the newly acquired image.
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
    
    // If the swapchain is bad, recreate it (will likely cause frame hitch).
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapchain(&vulkan_info, &swapchain_info);
    } else if (result != VK_SUCCESS) {
        ExitWithError("Unable to present swapchain image!");
    }
    
    // Increment frame counter.
    swapchain_info.current_frame = (swapchain_info.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// TODO(Matt): Move this out of the rendering component.
void UpdateModels(double frame_delta)
{
    // TODO(Matt): hack here until we're using better simulation rules.
    uint32_t current_index = 0;
    for (uint32_t i = 0; i < arrlen(material_types); ++i) {
        MaterialLayout *material_type = &material_types[i];
        for (uint32_t j = 0; j < arrlen(material_type->materials); ++j) {
            Material *material = &material_type->materials[j];
            for (uint32_t k = 0; k < arrlen(material->models); ++k) {
                Model *model = &material->models[k];
                model->rot.z += (float)frame_delta * glm::radians(25.0f);
                model->ubo.model = glm::translate(glm::mat4(1.0f), initial_positions[current_index]);
                model->ubo.model = glm::yawPitchRoll(model->rot.x, model->rot.y, model->rot.z) * model->ubo.model;
                model->pos = glm::vec3(model->ubo.model[3].x, model->ubo.model[3].y, model->ubo.model[3].z);
                model->ubo.sun.direction = glm::vec4(0.7f, -0.2f, -1.0f, 0.0f);
                model->ubo.sun.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
                model->ubo.sun.specular = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
                model->ubo.sun.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);
                current_index++;
            }
        }
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
/*
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
*/
void OnWindowResized()
{
    RecreateSwapchain(&vulkan_info, &swapchain_info);
}

MaterialLayout CreateMaterialLayout()
{
    MaterialLayout layout = {};
    VkDescriptorSetLayoutBinding uniform_binding = {};
    uniform_binding.binding = 0;
    uniform_binding.descriptorCount = 1;
    uniform_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_binding.pImmutableSamplers = nullptr;
    uniform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    // Binding 1 is first sampler, for now.
    VkDescriptorSetLayoutBinding sampler_binding = {};
    sampler_binding.binding = 1;
    sampler_binding.descriptorCount = 1;
    sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_binding.pImmutableSamplers = nullptr;
    sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    // Binding 2 is second sampler, for now.
    VkDescriptorSetLayoutBinding bindings[] = {uniform_binding, sampler_binding};
    VkDescriptorSetLayoutCreateInfo descriptor_info = {};
    descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_info.bindingCount = 2;
    descriptor_info.pBindings = bindings;
    
    // Create descriptor set layouts.
    VkDescriptorSetLayout descriptor_layout;
    for (uint32_t i = 0; i < swapchain_info.image_count; ++i) {
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vulkan_info.logical_device, &descriptor_info, nullptr, &descriptor_layout));
        arrput(layout.descriptor_layouts, descriptor_layout);
    }
    VkPipelineLayoutCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_info.setLayoutCount = swapchain_info.image_count;
    pipeline_info.pSetLayouts = layout.descriptor_layouts;
    
    VK_CHECK_RESULT(vkCreatePipelineLayout(vulkan_info.logical_device, &pipeline_info, nullptr, &layout.pipeline_layout));
    return layout;
}

MaterialCreateInfo CreateDefaultMaterialInfo(const char *vert_file, const char *frag_file)
{
    MaterialCreateInfo result = {};
    
    result.stage_count = (frag_file) ? 2 : 1;
    result.shader_stages = (VkPipelineShaderStageCreateInfo *)malloc(sizeof(VkPipelineShaderStageCreateInfo) * result.stage_count);
    result.shader_modules = (VkShaderModule *)malloc(sizeof(VkShaderModule) * result.stage_count);
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
    result.shader_stages[0] = vert_create_info;
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
        result.shader_stages[1] = frag_create_info;
    }
    
    result.input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    result.input_info.vertexBindingDescriptionCount = 1;
    result.input_info.vertexAttributeDescriptionCount = 6;
    
    result.binding_description.binding = 0;
    result.binding_description.stride = sizeof(Vertex);
    result.binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    result.attribute_descriptions[0].binding = 0;
    result.attribute_descriptions[0].location = 0;
    result.attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    result.attribute_descriptions[0].offset = 0;
    
    result.attribute_descriptions[1].binding = 0;
    result.attribute_descriptions[1].location = 1;
    result.attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    result.attribute_descriptions[1].offset = 12;
    
    result.attribute_descriptions[2].binding = 0;
    result.attribute_descriptions[2].location = 2;
    result.attribute_descriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    result.attribute_descriptions[2].offset = 24;
    
    result.attribute_descriptions[3].binding = 0;
    result.attribute_descriptions[3].location = 3;
    result.attribute_descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    result.attribute_descriptions[3].offset = 40;
    
    result.attribute_descriptions[4].binding = 0;
    result.attribute_descriptions[4].location = 4;
    result.attribute_descriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
    result.attribute_descriptions[4].offset = 48;
    
    result.attribute_descriptions[5].binding = 0;
    result.attribute_descriptions[5].location = 5;
    result.attribute_descriptions[5].format = VK_FORMAT_R32G32_SFLOAT;
    result.attribute_descriptions[5].offset = 56;
    
    result.assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    result.assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    result.assembly_info.primitiveRestartEnable = VK_FALSE;
    
    result.viewport.x = 0.0f;
    result.viewport.y = 0.0f;
    result.viewport.width = (float)swapchain_info.extent.width;
    result.viewport.height = (float)swapchain_info.extent.height;
    result.viewport.minDepth = 0.0f;
    result.viewport.maxDepth = 1.0f;
    
    result.scissor.offset = {0, 0};
    result.scissor.extent = swapchain_info.extent;
    
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
    
    return result;
}

void AddMaterial(MaterialCreateInfo *material_info, uint32_t material_type, VkRenderPass render_pass, uint32_t sub_pass)
{
    material_info->input_info.pVertexBindingDescriptions = &material_info->binding_description;
    material_info->input_info.pVertexAttributeDescriptions = material_info->attribute_descriptions;
    material_info->viewport_info.pViewports = &material_info->viewport;
    material_info->viewport_info.pScissors = &material_info->scissor;
    material_info->blend_info.pAttachments = &material_info->blend;
    
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
    pipeline_info.layout = material_types[material_type].pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = sub_pass;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    
    Material material = {};
    material.type = material_type;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(vulkan_info.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &material.pipeline));
    for (uint32_t i = 0; i < material_info->stage_count; ++i) {
        vkDestroyShaderModule(vulkan_info.logical_device, material_info->shader_modules[i], nullptr);
    }
    
    free(material_info->shader_stages);
    free(material_info->shader_modules);
    arrput(material_types[material.type].materials, material);
}

void CreateMaterials()
{
    MaterialCreateInfo material_info;
    arrput(material_types, CreateMaterialLayout());
    material_info = CreateDefaultMaterialInfo("shaders/vert.spv", "shaders/frag.spv");
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("shaders/vert2.spv", "shaders/frag2.spv");
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("shaders/stencil_vert.spv", nullptr);
    material_info.raster_info.cullMode = VK_CULL_MODE_NONE;
    material_info.depth_stencil.depthTestEnable = VK_FALSE;
    material_info.depth_stencil.depthWriteEnable = VK_FALSE;
    material_info.depth_stencil.stencilTestEnable = VK_TRUE;
    material_info.depth_stencil.back = {};
    material_info.depth_stencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
    material_info.depth_stencil.back.failOp = VK_STENCIL_OP_REPLACE;
    material_info.depth_stencil.back.depthFailOp = VK_STENCIL_OP_REPLACE;
    material_info.depth_stencil.back.passOp = VK_STENCIL_OP_REPLACE;
    material_info.depth_stencil.back.compareMask = 0xff;material_info.depth_stencil.back.writeMask = 0xff;
    material_info.depth_stencil.back.reference = 1;
    material_info.depth_stencil.front = material_info.depth_stencil.back;
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("shaders/outline_vert.spv", "shaders/outline_frag.spv");
    material_info.raster_info.cullMode = VK_CULL_MODE_NONE;
    material_info.depth_stencil.depthTestEnable = VK_FALSE;
    material_info.depth_stencil.depthWriteEnable = VK_FALSE;
    material_info.depth_stencil.stencilTestEnable = VK_TRUE;
    material_info.depth_stencil.back = {};
    material_info.depth_stencil.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
    material_info.depth_stencil.back.failOp = VK_STENCIL_OP_KEEP;
    material_info.depth_stencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
    material_info.depth_stencil.back.passOp = VK_STENCIL_OP_REPLACE;
    material_info.depth_stencil.back.compareMask = 0xff;
    material_info.depth_stencil.back.writeMask = 0xff;
    material_info.depth_stencil.back.reference = 1;
    material_info.depth_stencil.front = material_info.depth_stencil.back;
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("shaders/text_vert.spv", "shaders/text_frag.spv");
    material_info.blend.blendEnable = VK_TRUE;
    material_info.blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    material_info.blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    material_info.blend.colorBlendOp = VK_BLEND_OP_ADD;
    material_info.blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    material_info.blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    material_info.blend.alphaBlendOp = VK_BLEND_OP_ADD;
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("shaders/fill_vcolor_vert.spv", "shaders/fill_vcolor_frag.spv");
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
}

void InitializeSceneResources()
{
    // Load fonts and textures.
    // TODO(Matt): Move texture/font initialization somewhere else.
    texture = LoadTexture(&vulkan_info, "textures/proto.jpg", 4, true);
    font = LoadBitmapFont(&vulkan_info, "fonts/Hind-Regular.ttf", 0, 4);
}
void InitializeScene()
{
    // Throw some boxes in the scene.
    AddToScene(CreateBox({-0.3f, -0.3f, -0.3f}, {0.5f, 0.5f, 0.5f}, 0, 0));
    AddToScene(CreateBox({-0.3f, -0.3f, -0.3f}, {0.5f, 0.5f, 0.5f}, 0, 1));
    
    // Add screen-space elements.
    // TODO(Matt): Move screen-space drawing out of the "scene" hierarchy.
    // It should probably live on its own.
    AddToScene(CreateDebugQuad2D({0.0f, 0.0f}, {1.0f, 0.25f}, 0, 5, {1.0f, 0.0f, 0.0f, 1.0f}, false));
    
    AddToScene(CreateText("This is some text.", &font, {0.2f, 0.5f}));
}

void DestroyMaterials()
{
    for (uint32_t i = 0; i < arrlen(material_types); ++i) {
        for (uint32_t j = 0; j < arrlen(material_types[i].materials); ++j) {
            vkDestroyPipeline(vulkan_info.logical_device, material_types[i].materials[j].pipeline, nullptr);
            // TODO(Matt): Put model destruction here once it's moved.
        }
        for (uint32_t j = 0; j < swapchain_info.image_count; ++j) {
            vkDestroyDescriptorSetLayout(vulkan_info.logical_device, material_types[i].descriptor_layouts[j], nullptr);
        }
        vkDestroyPipelineLayout(vulkan_info.logical_device, material_types[i].pipeline_layout, nullptr);
        arrfree(material_types[i].materials);
        arrfree(material_types[i].descriptor_layouts);
    }
    arrfree(material_types);
}

void DestroyScene() {
    for (uint32_t i = 0; i < arrlen(material_types); ++i) {
        for (uint32_t j = 0; j < arrlen(material_types[i].materials); ++j) {
            for (uint32_t k = 0; k < arrlen(material_types[i].materials[j].models); ++k) {
                DestroyModel(&material_types[i].materials[j].models[k], &vulkan_info);
            }
            arrfree(material_types[i].materials[j].models);
        }
    }
}

void AddToScene(Model model)
{
    CreateVertexBuffer(&model);
    CreateIndexBuffer(&model);
    CreateUniformBuffers(&model);
    CreateDescriptorSets(&model);
    arrput(material_types[model.material_type].materials[model.shader_id].models, model);
}

const VulkanInfo *GetRenderInfo()
{
    return &vulkan_info;
}

const SwapchainInfo *GetSwapchainInfo()
{
    return &swapchain_info;
}

uint32_t GetUniformCount()
{
    return swapchain_info.image_count;
}

void DestroySceneResources()
{
    DestroyFont(&vulkan_info, &font);
    DestroyTexture(&vulkan_info, &texture);
}