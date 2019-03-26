// TODO(Matt): There are a few platform specific bits lingering in here.
// TODO(Matt): Use a texture list, and have models which use textures
// specify which textures belong in which samplers.
// TODO(Matt): Re-handle the way that uniforms are updated. It's hacky.

#include "RenderBase.h"

static VulkanInfo vulkan_info = {};
static SwapchainInfo swapchain_info = {};
BitmapFont font = {};
Texture *textures = nullptr;
MaterialLayout *material_types = nullptr;

DescriptorLayout descriptor_layout_new = {};
VkBuffer *uniform_buffers_new = nullptr;
VkDeviceMemory *uniform_buffers_memory_new = nullptr;
UniformBuffer uniforms = {};
VkDescriptorSet *descriptor_sets_new = nullptr;

Model_Separate_Data **selected_models = nullptr;
// TODO(Matt): Refactor these.
Camera::Camera camera = {};

char *ReadShaderFile(const char *path, u32 *length)
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
    CreateGlobalUniformBuffers();
    InitializeSceneResources();
    CreateDescriptorSets();
    InitializeScene();
}

void ShutdownRenderer()
{
    DestroySwapchain(&vulkan_info, &swapchain_info);
    ShutdownVulkan(&vulkan_info, &swapchain_info);
}

static VkShaderModule CreateShaderModule(char *code, u32 length)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = length;
    create_info.pCode = reinterpret_cast<u32 *>(code);
    
    VkShaderModule module;
    if (vkCreateShaderModule(vulkan_info.logical_device, &create_info, nullptr, &module) != VK_SUCCESS)
    {
        std::cerr << "Unable to create shader module!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return module;
}
/*
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
    VkDeviceSize buffer_size = sizeof(u32) * model->index_count;
    
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
*/
void CreateDescriptorSets()
{
    arrsetlen(descriptor_sets_new, swapchain_info.image_count);
    VkDescriptorSetAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = vulkan_info.descriptor_pool;
    allocate_info.descriptorSetCount = swapchain_info.image_count;
    allocate_info.pSetLayouts = descriptor_layout_new.descriptor_layouts;
    //model->descriptor_sets = (VkDescriptorSet *)malloc(sizeof(VkDescriptorSet) * model->uniform_count);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkan_info.logical_device, &allocate_info, descriptor_sets_new));
    
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        VkDescriptorBufferInfo descriptor_info = {};
        descriptor_info.buffer = uniform_buffers_new[i];
        descriptor_info.offset = 0;
        descriptor_info.range = VK_WHOLE_SIZE;
        VkWriteDescriptorSet uniform_write = {};
        uniform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniform_write.dstSet = descriptor_sets_new[i];
        uniform_write.dstBinding = 0;
        uniform_write.dstArrayElement = 0;
        uniform_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        // TODO(Matt): Hardcode. Should be num render passes.
        uniform_write.descriptorCount = 1;
        uniform_write.pBufferInfo = &descriptor_info;
        vkUpdateDescriptorSets(vulkan_info.logical_device, 1, &uniform_write, 0, nullptr);
        UpdateTextureDescriptors(descriptor_sets_new[i]);
    }
}

void RecordPrimaryCommand(u32 image_index)
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
    vkCmdBindDescriptorSets(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material_types[0].pipeline_layout, 0, 1, &descriptor_sets_new[image_index], 1, offsets);
    // For each material type.
    for (u32 i = 0; i < arrlen(material_types); ++i) {
        MaterialLayout *material_type = &material_types[i];
        // For each material of a given type.
        for (u32 j = 0; j < arrlen(material_type->materials); ++j) {
            Material *material = &material_type->materials[j];
            
            // Bind the material pipeline.
            vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
            
            // For each model of a given material.
            for (u32 k = 0; k < arrlen(material->models); ++k) {
                // Model *model = &material->models[k];
                Model_Separate_Data *model = &material->models[k];
                
                // Bind the vertex, index, and uniform buffers.
                VkBuffer vertex_buffers[] = {model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer};
                vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 7, vertex_buffers, model->model_data->attribute_offsets);
                vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], model->index_buffer, 0, VK_INDEX_TYPE_UINT32);
                PushConstantBlock push_block = {};
                push_block.draw_index = model->uniform_index;
                push_block.scalar_parameters[0] = 1;
                push_block.scalar_parameters[1] = 2;
                push_block.scalar_parameters[2] = 3;
                push_block.scalar_parameters[3] = 4;
                push_block.scalar_parameters[4] = 5;
                push_block.scalar_parameters[5] = 6;
                push_block.scalar_parameters[6] = 7;
                vkCmdPushConstants(swapchain_info.primary_command_buffers[image_index], material_type->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBlock), (void *)&push_block);
                // Draw the model.
                vkCmdDrawIndexed(swapchain_info.primary_command_buffers[image_index], model->index_count, 1, 0, 0, 0);
            }
        }
    }
    
    // Do post process for outlines.
    // NOTE(Matt): Outlines are done in two passes - one to draw selected
    // into stencil buffer, and one to read stencil buffer for outlines.
    for (u32 outline_stage = 2; outline_stage <= 3; ++outline_stage) {
        if (arrlen(selected_models) == 0) {
            break;
        }
        
        // TODO(Matt): Currently post process stuff is hard-coded. Should
        // maybe use a separate set of materials entirely for post process.
        // Bind the stenciling pipeline.
        vkCmdBindPipeline(swapchain_info.primary_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, material_types[0].materials[outline_stage].pipeline);
        // For each selected model.
        for (u32 i = 0; i < arrlen(selected_models); ++i) {
            // Bind vertex and index buffers, and uniforms.
            Model_Separate_Data *model = selected_models[i];
            
            // Bind the vertex, index, and uniform buffers.
            VkBuffer vertex_buffers[] = {model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer, model->vertex_buffer};
            vkCmdBindVertexBuffers(swapchain_info.primary_command_buffers[image_index], 0, 7, vertex_buffers, model->model_data->attribute_offsets);
            vkCmdBindIndexBuffer(swapchain_info.primary_command_buffers[image_index], selected_models[i]->index_buffer, 0, VK_INDEX_TYPE_UINT32);
            PushConstantBlock push_block = {};
            push_block.draw_index = model->uniform_index;
            push_block.scalar_parameters[0] = 1;
            push_block.scalar_parameters[1] = 2;
            push_block.scalar_parameters[2] = 3;
            push_block.scalar_parameters[3] = 4;
            push_block.scalar_parameters[4] = 5;
            push_block.scalar_parameters[5] = 6;
            push_block.scalar_parameters[6] = 7;
            vkCmdPushConstants(swapchain_info.primary_command_buffers[image_index], material_types[0].pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantBlock), (void *)&push_block);
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
    u32 image_index;
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
    UpdateUniforms(image_index);
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
void UpdatePrePhysics(double delta)
{
    if (GetInputMode() == VIEWPORT) {
        float forward_axis = GetForwardAxis();
        if (fabs(forward_axis) > 0.01f) Camera::MoveForward(&camera, forward_axis * (float)delta);
        float right_axis = GetRightAxis();
        if (fabs(right_axis) > 0.01f) Camera::MoveRight(&camera, right_axis * (float)delta);
        
        float up_axis = GetUpAxis();
        if (fabs(up_axis) > 0.01f) Camera::MoveUp(&camera, up_axis * (float)delta);
        //Camera::Move(&camera, forward_axis, right_axis, up_axis, (float) delta);
        s32 x, y;
        float x_delta, y_delta;
        PlatformGetCursorDelta(&x, &y);
        
        x_delta = (float)x;
        y_delta = (float)y;
        float yaw = x_delta * -0.25f * (float)delta;
        float pitch = y_delta * -0.25f * (float)delta;
        Camera::AddYaw(&camera, yaw);
        Camera::AddPitch(&camera, pitch);
    }
    
    PerFrameUniformObject *per_frame = GetPerFrameUniform();
    per_frame->view_position = glm::vec4(camera.location, 1.0f);
    per_frame->view = Camera::GetViewTransform(&camera);
    per_frame->projection = Camera::GetProjectionTransform(&camera);
    
    PerPassUniformObject *per_pass = GetPerPassUniform();
    per_pass->sun.direction = glm::vec4(0.7f, -0.2f, -1.0f, 0.0f);
    per_pass->sun.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
    per_pass->sun.specular = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    per_pass->sun.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);
    // TODO(Matt): hack here until we're using better simulation rules.
    u32 current_index = 0;
    for (u32 i = 0; i < arrlen(material_types); ++i) {
        MaterialLayout *material_type = &material_types[i];
        for (u32 j = 0; j < arrlen(material_type->materials); ++j) {
            Material *material = &material_type->materials[j];
            for (u32 k = 0; k < arrlen(material->models); ++k) {
                Model_Separate_Data *model = &material->models[k];
                PerDrawUniformObject *ubo = GetPerDrawUniform(current_index);
                //model->rot.z += (float)delta * glm::radians(25.0f);
                ubo->model = glm::scale(glm::mat4(1.0f), model->scl);
                ubo->model = glm::rotate(ubo->model, model->rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
                ubo->model = glm::rotate(ubo->model, model->rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
                ubo->model = glm::rotate(ubo->model, model->rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
                ubo->model = glm::translate(ubo->model, model->pos);
                current_index++;
            }
        }
    }
}

// TODO(Matt): Move this out of the rendering component.
void UpdatePhysics(double frame_delta)
{
    
}

// TODO(Matt): Move this out of the rendering component.
void UpdatePostPhysics(double delta)
{
    
}

// TODO(Matt): Move this out of the rendering component.
void UpdatePostRender(double delta)
{
    
}

// TODO(Matt): Figure out uniforms in general.
void UpdateUniforms(u32 image_index)
{
    void *data;
    vkMapMemory(vulkan_info.logical_device, uniform_buffers_memory_new[image_index], 0, uniforms.buffer_size, 0, &data);
    memcpy(data, uniforms.buffer, uniforms.buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, uniform_buffers_memory_new[image_index]);
}

void SelectObject(s32 mouse_x, s32 mouse_y, bool accumulate)
{
    // If we are not doing multi-select, reset selected count to zero.
    if (!accumulate) arrfree(selected_models);
    // Initialize the  minimum distance.
    float min_dist = INFINITY;
    Model_Separate_Data *selection = nullptr;
    // Iterate through all objects in the scene.
    for (u32 i = 0; i < arrlenu(material_types); ++i) {
        MaterialLayout *material_type = &material_types[i];
        for (u32 j = 0; j < arrlenu(material_type->materials); ++j) {
            Material *material = &material_type->materials[j];
            for (u32 k = 0; k < arrlenu(material->models); ++k) {
                Model_Separate_Data *model = &material->models[k];
                // If the model has hit testing disabled, skip it.
                if (!model->hit_test_enabled) continue;
                
                // Otherwise, perform a ray-box test with the object bounds.
                float hit_dist;
                PerFrameUniformObject *per_frame = GetPerFrameUniform();
                Ray ray = ScreenPositionToWorldRay(mouse_x, mouse_y, swapchain_info.extent.width, swapchain_info.extent.height, per_frame->view, per_frame->projection, 1000.0f);
                glm::vec3 intersection;
                if (RaycastAgainstModelBounds(ray, model, &intersection)) {
                    hit_dist = glm::distance2(ray.origin, intersection);
                    // If the raycast hits AND is closer than the previous hit
                    // we can update the selection.
                    if (hit_dist < min_dist) {
                        min_dist = hit_dist;
                        selection = model;
                    }
                }
            }
        }
    }
    
    // If any object was hit
    if (selection) {
        // Check if this box is already selected.
        s32 existing_index = -1;
        for (u32 i = 0; i < arrlenu(selected_models); ++i) {
            if (selected_models[i] == selection) {
                // If so, save the existing index and quit checking.
                existing_index = i;
                break;
            }
        }
        // If we are doing multi-select and this box was already selected, deselect it.
        if (existing_index >= 0 && accumulate) {
            arrdelswap(selected_models, existing_index);
        } else {
            arrput(selected_models, selection);
        }
    }
}

void OnWindowResized()
{
    RecreateSwapchain(&vulkan_info, &swapchain_info);
}

void CreateDescriptorLayout()
{
    CreateSamplers(&descriptor_layout_new);
    
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
    sampler_binding.pImmutableSamplers = descriptor_layout_new.samplers;
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
        arrput(descriptor_layout_new.descriptor_layouts, descriptor_layout);
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
        u32 frag_length = 0;
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

void AddMaterial(MaterialCreateInfo *material_info, u32 material_type, VkRenderPass render_pass, u32 sub_pass)
{
    material_info->input_info.pVertexBindingDescriptions = material_info->binding_description;
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
    for (u32 i = 0; i < material_info->stage_count; ++i) {
        vkDestroyShaderModule(vulkan_info.logical_device, material_info->shader_modules[i], nullptr);
    }
    
    free(material_info->shader_stages);
    free(material_info->shader_modules);
    arrput(material_types[material.type].materials, material);
}

void CreateMaterials()
{
    CreateDescriptorLayout();
    MaterialCreateInfo material_info;
    arrput(material_types, CreateMaterialLayout());
    material_info = CreateDefaultMaterialInfo("resources/shaders/vert.spv", "resources/shaders/frag.spv");
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/vert2.spv", "resources/shaders/frag2.spv");
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/stencil_vert.spv", nullptr);
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
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/outline_vert.spv", "resources/shaders/outline_frag.spv");
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
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/text_vert.spv", "resources/shaders/text_frag.spv");
    material_info.blend.blendEnable = VK_TRUE;
    material_info.blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    material_info.blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    material_info.blend.colorBlendOp = VK_BLEND_OP_ADD;
    material_info.blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    material_info.blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    material_info.blend.alphaBlendOp = VK_BLEND_OP_ADD;
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/fill_vcolor_vert.spv", "resources/shaders/fill_vcolor_frag.spv");
    AddMaterial(&material_info, 0, swapchain_info.renderpass, 0);
}

// offset is the offset into buffer memory the VkBuffer will be written to
// flags will probably be either VK_BUFFER_USAGE_VERTEX_BUFFER_BIT or VK_BUFFER_USAGE_INDEX_BUFFER_BIT
void CreateModelBuffer(VkDeviceSize buffer_size, void* buffer_data, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkBufferUsageFlagBits flags)
{
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    
    void *data;
    vkMapMemory(vulkan_info.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, buffer_data, (size_t)buffer_size);
    vkUnmapMemory(vulkan_info.logical_device, staging_buffer_memory);
    
    CreateBuffer(&vulkan_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *buffer_memory);
    
    CopyBuffer(&vulkan_info, staging_buffer, *buffer, buffer_size);
    
    vkDestroyBuffer(vulkan_info.logical_device, staging_buffer, nullptr);
    vkFreeMemory(vulkan_info.logical_device, staging_buffer_memory, nullptr);
}

void InitializeSceneResources()
{
    // Load fonts and textures.
    // TODO(Matt): Move texture/font initialization somewhere else.
    arrput(textures, LoadTexture(&vulkan_info, "resources/textures/proto.jpg", 4, true));
    //font = LoadBitmapFont(&vulkan_info, "resources/fonts/Hind-Regular.ttf", 0, 4);
    //arrput(textures, font.texture);
}
void InitializeScene()
{
    camera.location = glm::vec3(-2.0f, 0.0f, 0.0f);
    // Throw some boxes in the scene.
    //AddToScene(CreateBoxNonInterleaved({-0.3f, -0.3f, -0.3f}, {0.5f, 0.5f, 0.5f}, 0, 0));
    Model_Separate_Data* model = (Model_Separate_Data*)malloc(sizeof(Model_Separate_Data));
    //arrsetlen(object_uniforms_new, arrlenu(object_uniforms_new) + 1);
    //u32 object_index = (u32)arrlen(object_uniforms_new) - 1;
    EModelLoadResult result = LoadGTLFModel(std::string(""), *model, GetPerDrawUniform(uniforms.object_count), 0, 0, uniforms.object_count);
    if (result == MODEL_LOAD_RESULT_SUCCESS) {
        uniforms.object_count++;
        AddToScene(*model);
    } else printf("FAILURE TO LOAD MODEL\n");
    // Model_Separate_Data model = CreateBoxNonInterleaved({-0.3f, -0.3f, -0.3f}, {0.5f, 0.5f, 0.5f}, 0, 0);
    // AddToScene(model);
    
    // DestroyModelSeparateDataTest(&m, &vulkan_info);
    
    // Add screen-space elements.
    // TODO(Matt): Move screen-space drawing out of the "scene" hierarchy.
    // It should probably live on its own.
    // AddToScene(CreateDebugQuad2D({0.0f, 0.0f}, {1.0f, 0.25f}, 0, 5, {1.0f, 0.0f, 0.0f, 1.0f}, false));
    
    // AddToScene(CreateText("This is some text.", &font, {0.2f, 0.5f}));
}

void DestroyMaterials()
{
    for (u32 i = 0; i < arrlen(material_types); ++i) {
        for (u32 j = 0; j < arrlen(material_types[i].materials); ++j) {
            vkDestroyPipeline(vulkan_info.logical_device, material_types[i].materials[j].pipeline, nullptr);
        }
        
        vkDestroyPipelineLayout(vulkan_info.logical_device, material_types[i].pipeline_layout, nullptr);
        arrfree(material_types[i].materials);
    }
    arrfree(material_types);
    vkDestroyDescriptorSetLayout(vulkan_info.logical_device, descriptor_layout_new.descriptor_layouts[0], nullptr);
    for (u32 i = 0; i < MATERIAL_SAMPLER_COUNT; ++i) {
        vkDestroySampler(vulkan_info.logical_device, descriptor_layout_new.samplers[i], nullptr);
    }
    arrfree(descriptor_layout_new.descriptor_layouts);
}

void DestroyScene() {
    for (u32 i = 0; i < arrlen(material_types); ++i) {
        for (u32 j = 0; j < arrlen(material_types[i].materials); ++j) {
            for (u32 k = 0; k < arrlen(material_types[i].materials[j].models); ++k) {
                DestroyModelSeparateDataTest(&material_types[i].materials[j].models[k], &vulkan_info);
            }
            arrfree(material_types[i].materials[j].models);
        }
    }
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        vkDestroyBuffer(vulkan_info.logical_device, uniform_buffers_new[i], nullptr);
        vkFreeMemory(vulkan_info.logical_device, uniform_buffers_memory_new[i], nullptr);
    }
    arrfree(uniform_buffers_memory_new);
    arrfree(uniform_buffers_new);
    uniforms.object_count = 0;
}

void AddToScene(Model_Separate_Data model)
{
    VkDeviceSize v_len = (model.model_data->memory_block_size - sizeof(u32) * model.index_count);
    
    CreateModelBuffer(v_len, model.model_data->position, &model.vertex_buffer, &model.vertex_buffer_memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    CreateModelBuffer(sizeof(u32) * model.index_count, model.model_data->indices, &model.index_buffer, &model.index_buffer_memory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
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

u32 GetUniformCount()
{
    return swapchain_info.image_count;
}

void DestroySceneResources()
{
    //DestroyFont(&vulkan_info, &font);
    for (u32 i = 0; i < arrlen(textures); ++i) {
        DestroyTexture(&vulkan_info, &textures[i]);
    }
    arrfree(textures);
}

void UpdateTextureDescriptors(VkDescriptorSet descriptor_set)
{
    VkDescriptorImageInfo image_infos[MAX_TEXTURES];
    for (u32 i = 0; i < MAX_TEXTURES; ++i) {
        image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_infos[i].imageView = textures[(i >= arrlen(textures)) ? 0 : i].image_view;
    }
    
    VkWriteDescriptorSet sampler_write = {};
    sampler_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    sampler_write.dstSet = descriptor_set;
    sampler_write.dstBinding = 2;
    sampler_write.dstArrayElement = 0;
    sampler_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    sampler_write.descriptorCount = MAX_TEXTURES;
    sampler_write.pImageInfo = image_infos;
    vkUpdateDescriptorSets(vulkan_info.logical_device, 1, &sampler_write, 0, nullptr);
}

void CreateSamplers(DescriptorLayout *layout)
{
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
    // TODO(Matt): Hardcode.
    sampler_create_info.maxLod = MAX_SAMPLER_LOD;
    
    for (u32 i = 0; i < MATERIAL_SAMPLER_COUNT; ++i) {
        VK_CHECK_RESULT(vkCreateSampler(vulkan_info.logical_device, &sampler_create_info, nullptr, &layout->samplers[i]));
    }
}

// TODO(Matt): This should put one descriptor per render pass into the same
// buffer. Currently there's only one render pass.
void CreateGlobalUniformBuffers()
{
    // NOTE(Matt): We don't yet need to worry about buffer alignment,
    // because we're only using one render pass ATM.
    //VkPhysicalDeviceProperties properties;
    //vkGetPhysicalDeviceProperties(vulkan_info.physical_device, &properties);
    //u32 alignment = (u32)properties.limits.minUniformBufferOffsetAlignment;
    
    uniforms.per_pass_offset = sizeof(PerFrameUniformObject);
    uniforms.per_draw_offset = sizeof(PerPassUniformObject) + uniforms.per_pass_offset;
    uniforms.buffer_size = uniforms.per_draw_offset + sizeof(PerDrawUniformObject) * MAX_OBJECTS;
    
    arrsetlen(uniform_buffers_new, swapchain_info.image_count);
    arrsetlen(uniform_buffers_memory_new, swapchain_info.image_count);
    uniforms.buffer = (char *)malloc(uniforms.buffer_size);
    for (u32 i = 0; i < swapchain_info.image_count; ++i) {
        CreateBuffer(&vulkan_info, uniforms.buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers_new[i], uniform_buffers_memory_new[i]);
    }
}

PerDrawUniformObject *GetPerDrawUniform(u32 object_index)
{
    char *object = uniforms.buffer + uniforms.per_draw_offset + object_index * sizeof(PerDrawUniformObject);
    return (PerDrawUniformObject *)object;
}

PerFrameUniformObject *GetPerFrameUniform()
{
    return (PerFrameUniformObject *)uniforms.buffer;
}

PerPassUniformObject *GetPerPassUniform()
{
    return (PerPassUniformObject *)(uniforms.buffer + uniforms.per_pass_offset);
}
