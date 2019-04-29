// TODO(Matt): Use a texture list, and have models which use textures
// specify which textures belong in which samplers.
// TODO(Matt): Re-handle the way that uniforms are updated. It's hacky.

#include "RenderBase.h"

BitmapFont font = {};
Texture *textures = nullptr;
// MaterialLayout *material_types = nullptr;

DescriptorLayout descriptor_layout_new = {};
VkBuffer *uniform_buffers_new = nullptr;
VkDeviceMemory *uniform_buffers_memory_new = nullptr;
UniformBuffer uniforms = {};
VkDescriptorSet *descriptor_sets_new = nullptr;

SceneManager *scene_manager;

Model **selected_models = nullptr;
ModelInstanced bounding_boxes = {};

// TODO(Matt): Refactor these.
Camera::Camera camera = {};
Camera::Controller controller = {16.0f, 16.0f, 2.0f, 0.25f, glm::vec3(0.0f), glm::vec3(0.0f)};

// TODO(Matt): Move this to the platform layer.
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
    InitializeVulkan();
    InitializeScene();
}

void ShutdownRenderer()
{
    WaitDeviceIdle();
    DestroyScene();
    ShutdownVulkan();
}

void RecordRenderCommands(u32 image_index)
{
    // Camera::Frustum *frustum_planes = Camera::ExtractFrustumPlanes(camera, &Camera::GetViewTransform(&camera));
    Camera::Frustum *frustum_planes = Camera::ExtractFrustumPlanes(camera);
    
    // scene_manager->FrustumCull(frustum_planes);
    
    RenderSceneMaterial* rsm = scene_manager->GetVisibleData();
    
    Model* m = scene_manager->GetModelByIndex(0);
    
    u32 num = 0;
    
    CommandBeginRenderPass(image_index);
    // For each material type.
    for (u32 i = 0; i < arrlen(rsm); ++i) {
        MaterialLayout *material_type = scene_manager->GetMaterialLayout(rsm[i].mat_layout_idx);
        // For each material of a given type.
        for (u32 j = 0; j < arrlen(rsm[i].scene_materials); ++j) {
            Material *material = scene_manager->GetMaterial(rsm[i].mat_layout_idx, rsm[i].scene_materials[j].mat_idx);
            // Bind the material pipeline and set dynamic state.
            CommandBindPipeline(material->pipeline, image_index);
            
            // For each model of a given material.
            for (u32 k = 0; k < arrlen(rsm[i].scene_materials[j].model_idx); ++k) {
                ++num;
                // Model *model = &material->models[k];
                // rsm[i].scene_materials[j].model_idx[k] <- index into model list
                Model *model = scene_manager->GetModelByIndex(rsm[i].scene_materials[j].model_idx[k]);
                
                // Bind the vertex, index, and uniform buffers.
                CommandBindVertexBuffer(model->vertex_buffer, model->model_data->attribute_offsets, 7, image_index);
                CommandBindIndexBuffer(model->index_buffer, VK_INDEX_TYPE_UINT32, image_index);
                PushConstantBlock push_block = {};
                push_block.draw_index = model->uniform_index;
                CommandPushConstants(material_type->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &push_block, image_index);
                // Draw the model.
                CommandDrawIndexed(image_index, model->index_count, 1);
            }
        }
    }
    
    Material* material = scene_manager->GetMaterial(bounding_boxes.material_type, bounding_boxes.shader_id);
    CommandBindPipeline(material->pipeline, image_index);
    CommandBindVertexBuffer(bounding_boxes.vertex_buffer, bounding_boxes.attribute_offsets, 4, image_index);
    CommandBindIndexBuffer(bounding_boxes.index_buffer, VK_INDEX_TYPE_UINT32, image_index);
    CommandDrawIndexed(image_index, bounding_boxes.index_count, bounding_boxes.instance_count);
    
    //printf("Number of models rendered: %d\n", num);
    
    // Do post process for outlines.
    // NOTE(Matt): Outlines are done in two passes - one to draw selected
    // into stencil buffer, and one to read stencil buffer for outlines.
    for (u32 outline_stage = 3; outline_stage <= 4; ++outline_stage) {
        if (arrlen(selected_models) == 0) {
            break;
        }
        
        // TODO(Matt): Currently post process stuff is hard-coded. Should
        // maybe use a separate set of materials entirely for post process.
        // Bind the stenciling pipeline.
        MaterialLayout *material_type = scene_manager->GetMaterialLayout(0);
        CommandBindPipeline(material_type->materials[outline_stage].pipeline, image_index);
        // For each selected model.
        for (u32 i = 0; i < arrlen(selected_models); ++i) {
            // Bind vertex and index buffers, and uniforms.
            Model *model = selected_models[i];
            CommandBindVertexBuffer(model->vertex_buffer, model->model_data->attribute_offsets, 7, image_index);
            CommandBindIndexBuffer(model->index_buffer, VK_INDEX_TYPE_UINT32, image_index);
            // Bind the vertex, index, and uniform buffers.
            PushConstantBlock push_block = {};
            push_block.draw_index = model->uniform_index;
            CommandPushConstants(material_type->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &push_block, image_index);
            // Draw selected models.
            CommandDrawIndexed(image_index, model->index_count, 1);
        }
    }
    CommandEndRenderPass(image_index);
}

void DrawFrame()
{
    u32 image_index = WaitForNextImage();
    // Update uniforms to reflect new state of all scene objects.
    // TODO(Matt): Probably only update dynamic object uniforms - static
    // geometry won't change.
    UpdateUniforms(image_index);
    // Record draw calls and other work for this frame.
    RecordRenderCommands(image_index);
    
    SubmitRenderCommands(image_index);
    PresentNextFrame(image_index);
}

// TODO(Matt): Move this out of the rendering component.
void UpdatePrePhysics(double delta)
{
    if (GetInputMode() == VIEWPORT) {
        float forward_axis = GetForwardAxis();
        float right_axis = GetRightAxis();
        float up_axis = GetUpAxis();
        Camera::ApplyInput((float)delta, &controller, &camera, glm::vec3(forward_axis, right_axis, up_axis));
        s32 x, y;
        float x_delta, y_delta;
        PlatformGetCursorDelta(&x, &y);
        
        x_delta = (float)x;
        y_delta = (float)y;
        float yaw = x_delta * -0.005f;
        float pitch = y_delta * -0.005f;
        Camera::AddYaw(&camera, yaw);
        Camera::AddPitch(&camera, pitch);
    } else {
        Camera::ApplyInput((float)delta, &controller, &camera, glm::vec3(0.0f));
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
    u32 num_models = scene_manager->GetNumberOfModelsInScene();
    for (u32 i = 0; i < num_models; ++i) {
        Model* model = scene_manager->GetModelByIndex(i);
        PerDrawUniformObject *ubo = GetPerDrawUniform(current_index);
        ubo->model = glm::scale(glm::mat4(1.0f), model->scl);
        ubo->model = glm::rotate(ubo->model, model->rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
        ubo->model = glm::rotate(ubo->model, model->rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        ubo->model = glm::rotate(ubo->model, model->rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        ubo->model = glm::translate(ubo->model, model->pos);
        current_index++;
    }
    // for (u32 i = 0; i < arrlen(material_types); ++i) {
    //     MaterialLayout *material_type = &material_types[i];
    //     for (u32 j = 0; j < arrlen(material_type->materials); ++j) {
    //         Material *material = &material_type->materials[j];
    //         for (u32 k = 0; k < arrlen(material->models); ++k) {
    //             Model *model = &material->models[k];
    //             PerDrawUniformObject *ubo = GetPerDrawUniform(current_index);
    //             ubo->model = glm::scale(glm::mat4(1.0f), model->scl);
    //             ubo->model = glm::rotate(ubo->model, model->rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
    //             ubo->model = glm::rotate(ubo->model, model->rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
    //             ubo->model = glm::rotate(ubo->model, model->rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
    //             ubo->model = glm::translate(ubo->model, model->pos);
    //             current_index++;
    //         }
    //     }
    // }
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
    UpdateDeviceMemory(uniforms.buffer, uniforms.buffer_size, uniform_buffers_memory_new[image_index]);
}

void SelectObject(s32 mouse_x, s32 mouse_y, bool accumulate)
{
    // If we are not doing multi-select, reset selected count to zero.
    if (!accumulate) arrfree(selected_models);
    // Initialize the  minimum distance.
    float min_dist = INFINITY;
    Model *selection = nullptr;
    // Iterate through all objects in the scene.
    u32 len = scene_manager->GetNumberOfModelsInScene();
    for (u32 i = 0; i < len; ++i)
    {
        Model* model = scene_manager->GetModelByIndex(i);
        // If the model has hit testing disabled, skip it.
        if (!model->hit_test_enabled) continue;        
        // Otherwise, perform a ray-box test with the object bounds.
        float hit_dist;
        PerFrameUniformObject *per_frame = GetPerFrameUniform();
        u32 width, height;
        GetSwapchainExtent(&width, &height);
        Ray ray = ScreenPositionToWorldRay(mouse_x, mouse_y, width, height, per_frame->view, per_frame->projection, 1000.0f);
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
    // for (u32 i = 0; i < arrlenu(material_types); ++i) {
    //     MaterialLayout *material_type = &material_types[i];
    //     for (u32 j = 0; j < arrlenu(material_type->materials); ++j) {
    //         Material *material = &material_type->materials[j];
    //         for (u32 k = 0; k < arrlenu(material->models); ++k) {
    //             Model *model = &material->models[k];
    //             // If the model has hit testing disabled, skip it.
    //             if (!model->hit_test_enabled) continue;
    
    //             // Otherwise, perform a ray-box test with the object bounds.
    //             float hit_dist;
    //             PerFrameUniformObject *per_frame = GetPerFrameUniform();
    //             u32 width, height;
    //             GetSwapchainExtent(&width, &height);
    //             Ray ray = ScreenPositionToWorldRay(mouse_x, mouse_y, width, height, per_frame->view, per_frame->projection, 1000.0f);
    //             glm::vec3 intersection;
    //             if (RaycastAgainstModelBounds(ray, model, &intersection)) {
    //                 hit_dist = glm::distance2(ray.origin, intersection);
    //                 // If the raycast hits AND is closer than the previous hit
    //                 // we can update the selection.
    //                 if (hit_dist < min_dist) {
    //                     min_dist = hit_dist;
    //                     selection = model;
    //                 }
    //             }
    //         }
    //     }
    // }
    
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
    RecreateSwapchain();
}

void AddMaterial(MaterialCreateInfo *material_info, u32 material_type, VkRenderPass render_pass, u32 sub_pass)
{
    Material material = CreateMaterial(material_info, scene_manager->GetMaterialLayout(material_type)->pipeline_layout, 
                                       material_type, render_pass, sub_pass);
    
    scene_manager->AddMaterial(&material, material_type);
    //arrput(material_types[material.type].materials, material);
}

// TODO(Matt): Put materials in a hash table by friendly name. Engine default always goes in slot 0. If
// we access a non-default material and it isn't there (or is incompatible), return the default and log a warning.
// It's annoying referring to materials by index - index shifts around all the time.
void CreateMaterials()
{
    CreateDescriptorLayout(&descriptor_layout_new);
    MaterialCreateInfo material_info;
    
    u32 layout_idx = scene_manager->AddMaterialType(&CreateMaterialLayout());
    // arrput(material_types, CreateMaterialLayout());
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/engine_default_vert.spv", "resources/shaders/engine_default_frag.spv");
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/bounds_box_vert.spv", "resources/shaders/bounds_box_frag.spv");
    // We only need 4 bindings, although we need 7 attributes, as 4 of them are used for the transform.
    material_info.input_info.vertexBindingDescriptionCount = 4;
    // NOTE(Matt): We could fit an extra vec4 in here, because the bottom row of a transform is almost always
    // (0, 0, 0, 1). We would have to transpose the matrix and send rows 1-3, and transpose again in the shader.
    
    // Set up attribute descriptions.
    material_info.attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Color
    material_info.attribute_descriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT; // User Data
    material_info.attribute_descriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Transform (Column 0)
    material_info.attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Transform (Column 1)
    material_info.attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Transform (Column 2)
    material_info.attribute_descriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT; // Transform (Column 3)
    
    // Use four attribute slots to hold the mat4, all tied to the same binding.
    material_info.attribute_descriptions[3].binding = 3;
    material_info.attribute_descriptions[4].binding = 3;
    material_info.attribute_descriptions[5].binding = 3;
    material_info.attribute_descriptions[6].binding = 3;
    
    // Offsets are the location of the column in a mat4.
    material_info.attribute_descriptions[3].offset = 0 * sizeof(glm::vec4);
    material_info.attribute_descriptions[4].offset = 1 * sizeof(glm::vec4);
    material_info.attribute_descriptions[5].offset = 2 * sizeof(glm::vec4);
    material_info.attribute_descriptions[6].offset = 3 * sizeof(glm::vec4);
    
    // Instance Extent (pre-transform)
    material_info.binding_description[1].stride = sizeof(glm::vec4);
    material_info.binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    // Instance User Params (color, random, fade amount, etc)
    material_info.binding_description[2].stride = sizeof(glm::vec4);
    material_info.binding_description[2].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    // Instance Transform Column 1
    material_info.binding_description[3].stride = sizeof(glm::mat4);
    material_info.binding_description[3].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    
    // Use line rendering.
    material_info.assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    material_info.raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    // TODO(Matt): Update CommandBindPipeline to allow variable dynamic states, so we can pass line width.
    // material_info.dynamic_stage_count = 3;
    // material_info.dynamic_states[2] = VK_DYNAMIC_STATE_LINE_WIDTH;
    
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/vert.spv", "resources/shaders/frag.spv");
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/vert2.spv", "resources/shaders/frag2.spv");
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
    // TODO(Matt): Add some static initializers for common material configs, like "stencil test only" or whatever.
    // That should cut down on boilerplate like this.
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
    material_info.depth_stencil.back.compareMask = 0xff;
    material_info.depth_stencil.back.writeMask = 0xff;
    material_info.depth_stencil.back.reference = 1;
    material_info.depth_stencil.front = material_info.depth_stencil.back;
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
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
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/text_vert.spv", "resources/shaders/text_frag.spv");
    material_info.blend.blendEnable = VK_TRUE;
    material_info.blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    material_info.blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    material_info.blend.colorBlendOp = VK_BLEND_OP_ADD;
    material_info.blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    material_info.blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    material_info.blend.alphaBlendOp = VK_BLEND_OP_ADD;
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
    
    material_info = CreateDefaultMaterialInfo("resources/shaders/fill_vcolor_vert.spv", "resources/shaders/fill_vcolor_frag.spv");
    AddMaterial(&material_info, layout_idx, GetSwapchainRenderPass(), 0);
}

// offset is the offset into buffer memory the VkBuffer will be written to
// flags will probably be either VK_BUFFER_USAGE_VERTEX_BUFFER_BIT or VK_BUFFER_USAGE_INDEX_BUFFER_BIT
void CreateModelBuffer(VkDeviceSize buffer_size, void* buffer_data, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkBufferUsageFlagBits flags)
{
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
    CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *buffer_memory);
    UpdateDeviceMemory(buffer_data, buffer_size, staging_buffer_memory);
    CopyBuffer(staging_buffer, *buffer, buffer_size);
    DestroyDeviceBuffer(staging_buffer);
    FreeDeviceMemory(staging_buffer_memory);
}

internal void InitializeSceneResources()
{
    scene_manager = new SceneManager();
    
    CreateMaterials();
    CreateGlobalUniformBuffers();
    // Load fonts and textures.
    arrput(textures, LoadTexture("resources/textures/proto.jpg", 4, true));
    //font = LoadBitmapFont(&vulkan_info, "resources/fonts/Hind-Regular.ttf", 0, 4);
    //arrput(textures, font.texture);
    CreateDescriptorSets(uniform_buffers_new);
}
void InitializeScene()
{
    InitializeSceneResources();
    
    // Load scene config
    SceneSettings* scene = LoadSceneSettings("../../config/scene/default_scene.json"); 
    
    
    camera.location = glm::make_vec3(&scene->camera_data[0].position[0]);
    
    printf("There are %d models being read from the scene config.\n\n", scene->num_models);
    
    for (uint32_t i = 0; i < scene->num_models; ++i) 
    {
        SceneModelData* model_data = scene->model_data + i;
        Model* model = (Model*)malloc(sizeof(Model));
        EModelLoadResult result = LoadGTLFModel(model_data, *model, 
                                                GetPerDrawUniform(uniforms.object_count), 0, 2, uniforms.object_count);
        if (result == MODEL_LOAD_RESULT_SUCCESS) {
            uniforms.object_count++;
            // AddToScene(*model);
            VkDeviceSize v_len = (model->model_data->memory_block_size - sizeof(u32) * model->index_count);
            CreateModelBuffer(v_len, model->model_data->position, &model->vertex_buffer, &model->vertex_buffer_memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            CreateModelBuffer(sizeof(u32) * model->index_count, model->model_data->indices, &model->index_buffer, &model->index_buffer_memory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
            scene_manager->AddModel("", model);
        } else printf("FAILURE TO LOAD MODEL\n");
    }
    
    Model* m = scene_manager->GetModelByIndex(0);
    u32 num =  scene_manager->GetNumberOfModelsInScene();
    
    bounding_boxes.vertex_count = 8;
    bounding_boxes.index_count = 24;
    bounding_boxes.instance_count = num;
    bounding_boxes.material_type = 0;
    bounding_boxes.shader_id = 1;
    
    bounding_boxes.attribute_offsets[0] = 0;
    bounding_boxes.attribute_offsets[1] = sizeof(glm::vec3) * bounding_boxes.vertex_count;
    bounding_boxes.attribute_offsets[2] = bounding_boxes.attribute_offsets[1] + sizeof(glm::vec4) * bounding_boxes.instance_count;
    bounding_boxes.attribute_offsets[3] = bounding_boxes.attribute_offsets[2] + sizeof(glm::vec4) * bounding_boxes.instance_count;
    
    bounding_boxes.memory_size =
        bounding_boxes.attribute_offsets[3] + sizeof(glm::mat4) * bounding_boxes.instance_count;
    bounding_boxes.memory = malloc(bounding_boxes.memory_size);
    
    arrsetlen(bounding_boxes.indices, bounding_boxes.index_count);
    bounding_boxes.locations = (glm::vec3*)bounding_boxes.memory;
    bounding_boxes.colors = (glm::vec4*)((char*)bounding_boxes.memory + bounding_boxes.attribute_offsets[1]);
    bounding_boxes.user_data = (glm::vec4*)((char*)bounding_boxes.memory + bounding_boxes.attribute_offsets[2]);
    bounding_boxes.transforms = (glm::mat4*)((char*)bounding_boxes.memory + bounding_boxes.attribute_offsets[3]);
    
    // TODO(Matt): Move this into a static initializer for box primitives.
    bounding_boxes.indices[0] = 0;
    bounding_boxes.indices[1] = 1;
    bounding_boxes.indices[2] = 1;
    bounding_boxes.indices[3] = 2;
    bounding_boxes.indices[4] = 2;
    bounding_boxes.indices[5] = 3;
    bounding_boxes.indices[6] = 3;
    bounding_boxes.indices[7] = 0;
    bounding_boxes.indices[8] = 4;
    bounding_boxes.indices[9] = 5;
    bounding_boxes.indices[10] = 5;
    bounding_boxes.indices[11] = 6;
    bounding_boxes.indices[12] = 6;
    bounding_boxes.indices[13] = 7;
    bounding_boxes.indices[14] = 7;
    bounding_boxes.indices[15] = 4;
    bounding_boxes.indices[16] = 0;
    bounding_boxes.indices[17] = 4;
    bounding_boxes.indices[18] = 1;
    bounding_boxes.indices[19] = 5;
    bounding_boxes.indices[20] = 2;
    bounding_boxes.indices[21] = 6;
    bounding_boxes.indices[22] = 3;
    bounding_boxes.indices[23] = 7;
    
    bounding_boxes.locations[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    bounding_boxes.locations[1] = glm::vec3(1.0f, 0.0f, 0.0f);
    bounding_boxes.locations[2] = glm::vec3(1.0f, 1.0f, 0.0f);
    bounding_boxes.locations[3] = glm::vec3(0.0f, 1.0f, 0.0f);
    bounding_boxes.locations[4] = glm::vec3(0.0f, 0.0f, 1.0f);
    bounding_boxes.locations[5] = glm::vec3(1.0f, 0.0f, 1.0f);
    bounding_boxes.locations[6] = glm::vec3(1.0f, 1.0f, 1.0f);bounding_boxes.locations[7] = glm::vec3(0.0f, 1.0f, 1.0f);
    
    for (u32 i = 0; i < bounding_boxes.instance_count; ++i) {
        Model* model = scene_manager->GetModelByIndex(i);
        glm::vec4 color = glm::vec4(1.0f, 0.1f, 0.5f, 1.0f);
        glm::vec4 user_data = glm::vec4(1.0f);
        glm::mat4 transform = glm::mat4(1.0f);
        
        glm::vec3 location = model->bounds.min;
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = 2.0f * model->bounds.ext;
        
        transform = glm::scale(transform, scale);
        transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::translate(transform, location);
        
        bounding_boxes.colors[i] = color;
        bounding_boxes.user_data[i] = user_data;
        bounding_boxes.transforms[i] = transform;
    }
    
    void *a = bounding_boxes.memory;
    for (u32 i = 0; i < 8; ++i) {
        glm::vec3 v = *(glm::vec3*)((char*)a + i * sizeof(glm::vec3));
        printf("Vertex location was (%f, %f, %f)\n", v.x, v.y, v.z);
    }
    CreateModelBuffer(bounding_boxes.memory_size, bounding_boxes.memory, &bounding_boxes.vertex_buffer, &bounding_boxes.vertex_buffer_memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    CreateModelBuffer(sizeof(u32) * bounding_boxes.index_count, bounding_boxes.indices, &bounding_boxes.index_buffer, &bounding_boxes.index_buffer_memory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    
    // Delete scene config
    SaveSceneSettings(scene, "../../config/scene/default_scene.json");
    FreeSceneSettings(scene);
    
    // Load the OctTree
    float min[3] = {-100000, -100000, -100000};
    float max[3] = {100000, 100000, 100000};
    scene_manager->CreateSpatialHeirarchy(min, max);
    scene_manager->LoadOctTree();
    // scene_manager->PrintScene();
    
    
    // Add screen-space elements.
    // TODO(Matt): Move screen-space drawing out of the "scene" hierarchy.
    // It should probably live on its own.
}

// TODO(Dustin): Move to scene manager
internal void DestroySceneResources()
{
    // Destroy textures.
    for (u32 i = 0; i < arrlen(textures); ++i) {
        DestroyTexture(&textures[i]);
    }
    arrfree(textures);
    
    // Destroy materials.
    // for (u32 i = 0; i < arrlen(material_types); ++i) {
    //     for (u32 j = 0; j < arrlen(material_types[i].materials); ++j) {
    //         DestroyPipeline(material_types[i].materials[j].pipeline);
    //     }
    
    //     DestroyPipelineLayout(material_types[i].pipeline_layout);
    //     arrfree(material_types[i].materials);
    // }
    // arrfree(material_types);
    
    DestroyDescriptorSetLayout(descriptor_layout_new.descriptor_layouts[0]);
    arrfree(descriptor_layout_new.descriptor_layouts);
    for (u32 i = 0; i < MATERIAL_SAMPLER_COUNT; ++i) {
        DestroySampler(descriptor_layout_new.samplers[i]);
    }
}

void DestroyScene() {
    scene_manager->Shutdown();
    // for (u32 i = 0; i < arrlen(material_types); ++i) {
    //     for (u32 j = 0; j < arrlen(material_types[i].materials); ++j) {
    //         for (u32 k = 0; k < arrlen(material_types[i].materials[j].models); ++k) {
    //             DestroyModelSeparateDataTest(&material_types[i].materials[j].models[k]);
    //         }
    //         arrfree(material_types[i].materials[j].models);
    //     }
    // }
    for (u32 i = 0; i < GetSwapchainImageCount(); ++i) {
        DestroyDeviceBuffer(uniform_buffers_new[i]);
        FreeDeviceMemory(uniform_buffers_memory_new[i]);
    }
    arrfree(uniform_buffers_memory_new);
    arrfree(uniform_buffers_new);
    uniforms.object_count = 0;
    DestroySceneResources();
}

void AddToScene(Model model)
{
    VkDeviceSize v_len = (model.model_data->memory_block_size - sizeof(u32) * model.index_count);
    
    CreateModelBuffer(v_len, model.model_data->position, &model.vertex_buffer, &model.vertex_buffer_memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    CreateModelBuffer(sizeof(u32) * model.index_count, model.model_data->indices, &model.index_buffer, &model.index_buffer_memory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    // arrput(material_types[model.material_type].materials[model.shader_id].models, model);
    scene_manager->AddModel("", &model);
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
    
    UpdateDescriptorSet(sampler_write);
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
    
    arrsetlen(uniform_buffers_new, GetSwapchainImageCount());
    arrsetlen(uniform_buffers_memory_new, GetSwapchainImageCount());
    uniforms.buffer = (char *)malloc(uniforms.buffer_size);
    for (u32 i = 0; i < GetSwapchainImageCount(); ++i) {
        CreateBuffer(uniforms.buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers_new[i], uniform_buffers_memory_new[i]);
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
