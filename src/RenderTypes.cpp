
#include "RenderTypes.h"
#include "RenderBase.h"

#define STB_DS_IMPLEMENTATION
#include "stb/stb_ds.h"

void DestroyModel(Model *model, const VulkanInfo *vulkan_info)
{
    free(model->vertices);
    free(model->indices);
    for (uint32_t i = 0; i < model->uniform_count; ++i) {
        vkDestroyBuffer(vulkan_info->logical_device, model->uniform_buffers[i], nullptr);
        vkFreeMemory(vulkan_info->logical_device, model->uniform_buffers_memory[i], nullptr);
    }
    vkDestroyBuffer(vulkan_info->logical_device, model->vertex_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, model->vertex_buffer_memory, nullptr);
    vkDestroyBuffer(vulkan_info->logical_device, model->index_buffer, nullptr);
    vkFreeMemory(vulkan_info->logical_device, model->index_buffer_memory, nullptr);
    free(model->uniform_buffers);
    free(model->uniform_buffers_memory);
    free(model->descriptor_sets);
    model = nullptr;
}

Model CreateBox(glm::vec3 pos, glm::vec3 ext, uint32_t material_type, uint32_t shader_id)
{
    Model model;
    model.material_type = material_type;
    model.shader_id = shader_id;
    model.uniform_count = GetUniformCount();
    model.hit_test_enabled = true;
    model.vertex_count = 24;
    model.index_count = 36;
    model.vertices = (Vertex *)malloc(model.vertex_count * sizeof(Vertex));
    model.indices = (uint32_t *)malloc(sizeof(uint32_t) * model.index_count);
    model.vertices[ 0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    model.vertices[ 1].position = glm::vec3(0.0f, 0.0f, ext.z);
    model.vertices[ 2].position = glm::vec3(ext.x, 0.0f, ext.z);
    model.vertices[ 3].position = glm::vec3(ext.x, 0.0f, 0.0f);
    model.vertices[ 4].position = glm::vec3(ext.x, 0.0f, 0.0f);
    model.vertices[ 5].position = glm::vec3(ext.x, 0.0f, ext.z);
    model.vertices[ 6].position = glm::vec3(ext.x, ext.y, ext.z);
    model.vertices[ 7].position = glm::vec3(ext.x, ext.y, 0.0f);
    model.vertices[ 8].position = glm::vec3(ext.x, ext.y, 0.0f);
    model.vertices[ 9].position = glm::vec3(ext.x, ext.y, ext.z);
    model.vertices[10].position = glm::vec3(0.0f, ext.y, ext.z);
    model.vertices[11].position = glm::vec3(0.0f, ext.y, 0.0f);
    model.vertices[12].position = glm::vec3(0.0f, ext.y, 0.0f);
    model.vertices[13].position = glm::vec3(0.0f, ext.y, ext.z);
    model.vertices[14].position = glm::vec3(0.0f, 0.0f, ext.z);
    model.vertices[15].position = glm::vec3(0.0f, 0.0f, 0.0f);
    model.vertices[16].position = glm::vec3(0.0f, 0.0f, ext.z);
    model.vertices[17].position = glm::vec3(0.0f, ext.y, ext.z);
    model.vertices[18].position = glm::vec3(ext.x, ext.y, ext.z);
    model.vertices[19].position = glm::vec3(ext.x, 0.0f, ext.z);
    model.vertices[20].position = glm::vec3(0.0f, 0.0f, 0.0f);
    model.vertices[21].position = glm::vec3(ext.x, 0.0f, 0.0f);
    model.vertices[22].position = glm::vec3(ext.x, ext.y, 0.0f);
    model.vertices[23].position = glm::vec3(0.0f, ext.y, 0.0f);
    
    model.vertices[ 0].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    model.vertices[ 1].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    model.vertices[ 2].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    model.vertices[ 3].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    model.vertices[ 4].normal = glm::vec3(1.0f, 0.0f, 0.0f);
    model.vertices[ 5].normal = glm::vec3(1.0f, 0.0f, 0.0f);
    model.vertices[ 6].normal = glm::vec3(1.0f, 0.0f, 0.0f);
    model.vertices[ 7].normal = glm::vec3(1.0f, 0.0f, 0.0f);
    model.vertices[ 8].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    model.vertices[ 9].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    model.vertices[10].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    model.vertices[11].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    model.vertices[12].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    model.vertices[13].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    model.vertices[14].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    model.vertices[15].normal = glm::vec3(-1.0f, 0.0f, 0.0f);
    model.vertices[16].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    model.vertices[17].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    model.vertices[18].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    model.vertices[19].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    model.vertices[20].normal = glm::vec3(0.0f, 0.0f, -1.0f);
    model.vertices[21].normal = glm::vec3(0.0f, 0.0f, -1.0f);
    model.vertices[22].normal = glm::vec3(0.0f, 0.0f, -1.0f);
    model.vertices[23].normal = glm::vec3(0.0f, 0.0f, -1.0f);
    
    model.vertices[ 0].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 1].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 2].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 3].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 4].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 5].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 6].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 7].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 8].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[ 9].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[10].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[11].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[12].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[13].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[14].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[15].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[16].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[17].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[18].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[19].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[20].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[21].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[22].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    model.vertices[23].color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    
    model.vertices[ 0].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 1].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[ 2].uv0 = glm::vec2(0.0f, 1.0f);
    model.vertices[ 3].uv0 = glm::vec2(0.0f, 0.0f);
    model.vertices[ 4].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 5].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[ 6].uv0 = glm::vec2(0.0f, 1.0f);
    model.vertices[ 7].uv0 = glm::vec2(0.0f, 0.0f);
    model.vertices[ 8].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 9].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[10].uv0 = glm::vec2(0.0f, 1.0f);
    model.vertices[11].uv0 = glm::vec2(0.0f, 0.0f);
    model.vertices[12].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[13].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[14].uv0 = glm::vec2(0.0f, 1.0f);
    model.vertices[15].uv0 = glm::vec2(0.0f, 0.0f);
    model.vertices[16].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[17].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[18].uv0 = glm::vec2(0.0f, 1.0f);
    model.vertices[19].uv0 = glm::vec2(0.0f, 0.0f);
    model.vertices[20].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[21].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[22].uv0 = glm::vec2(0.0f, 1.0f);
    model.vertices[23].uv0 = glm::vec2(0.0f, 0.0f);
    
    
    model.vertices[ 0].uv1 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 1].uv1 = glm::vec2(1.0f, 1.0f);
    model.vertices[ 2].uv1 = glm::vec2(0.0f, 1.0f);
    model.vertices[ 3].uv1 = glm::vec2(0.0f, 0.0f);
    model.vertices[ 4].uv1 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 5].uv1 = glm::vec2(1.0f, 1.0f);
    model.vertices[ 6].uv1 = glm::vec2(0.0f, 1.0f);
    model.vertices[ 7].uv1 = glm::vec2(0.0f, 0.0f);
    model.vertices[ 8].uv1 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 9].uv1 = glm::vec2(1.0f, 1.0f);
    model.vertices[10].uv1 = glm::vec2(0.0f, 1.0f);
    model.vertices[11].uv1 = glm::vec2(0.0f, 0.0f);
    model.vertices[12].uv1 = glm::vec2(1.0f, 0.0f);
    model.vertices[13].uv1 = glm::vec2(1.0f, 1.0f);
    model.vertices[14].uv1 = glm::vec2(0.0f, 1.0f);
    model.vertices[15].uv1 = glm::vec2(0.0f, 0.0f);
    model.vertices[16].uv1 = glm::vec2(1.0f, 0.0f);
    model.vertices[17].uv1 = glm::vec2(1.0f, 1.0f);
    model.vertices[18].uv1 = glm::vec2(0.0f, 1.0f);
    model.vertices[19].uv1 = glm::vec2(0.0f, 0.0f);
    model.vertices[20].uv1 = glm::vec2(1.0f, 0.0f);
    model.vertices[21].uv1 = glm::vec2(1.0f, 1.0f);
    model.vertices[22].uv1 = glm::vec2(0.0f, 1.0f);
    model.vertices[23].uv1 = glm::vec2(0.0f, 0.0f);
    
    model.vertices[ 0].uv2 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 1].uv2 = glm::vec2(1.0f, 1.0f);
    model.vertices[ 2].uv2 = glm::vec2(0.0f, 1.0f);
    model.vertices[ 3].uv2 = glm::vec2(0.0f, 0.0f);
    model.vertices[ 4].uv2 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 5].uv2 = glm::vec2(1.0f, 1.0f);
    model.vertices[ 6].uv2 = glm::vec2(0.0f, 1.0f);
    model.vertices[ 7].uv2 = glm::vec2(0.0f, 0.0f);
    model.vertices[ 8].uv2 = glm::vec2(1.0f, 0.0f);
    model.vertices[ 9].uv2 = glm::vec2(1.0f, 1.0f);
    model.vertices[10].uv2 = glm::vec2(0.0f, 1.0f);
    model.vertices[11].uv2 = glm::vec2(0.0f, 0.0f);
    model.vertices[12].uv2 = glm::vec2(1.0f, 0.0f);
    model.vertices[13].uv2 = glm::vec2(1.0f, 1.0f);
    model.vertices[14].uv2 = glm::vec2(0.0f, 1.0f);
    model.vertices[15].uv2 = glm::vec2(0.0f, 0.0f);
    model.vertices[16].uv2 = glm::vec2(1.0f, 0.0f);
    model.vertices[17].uv2 = glm::vec2(1.0f, 1.0f);
    model.vertices[18].uv2 = glm::vec2(0.0f, 1.0f);
    model.vertices[19].uv2 = glm::vec2(0.0f, 0.0f);
    model.vertices[20].uv2 = glm::vec2(1.0f, 0.0f);
    model.vertices[21].uv2 = glm::vec2(1.0f, 1.0f);
    model.vertices[22].uv2 = glm::vec2(0.0f, 1.0f);
    model.vertices[23].uv2 = glm::vec2(0.0f, 0.0f);
    
    model.indices[0] = 0;
    model.indices[1] = 3;
    model.indices[2] = 2;
    model.indices[3] = 0;
    model.indices[4] = 2;
    model.indices[5] = 1;
    model.indices[6] = 4;
    model.indices[7] = 7;
    model.indices[8] = 6;
    model.indices[9] = 4;
    model.indices[10] = 6;
    model.indices[11] = 5;
    model.indices[12] = 8;
    model.indices[13] = 11;
    model.indices[14] = 10;
    model.indices[15] = 8;
    model.indices[16] = 10;
    model.indices[17] = 9;
    model.indices[18] = 12;
    model.indices[19] = 15;
    model.indices[20] = 14;
    model.indices[21] = 12;
    model.indices[22] = 14;
    model.indices[23] = 13;
    model.indices[24] = 16;
    model.indices[25] = 19;
    model.indices[26] = 18;
    model.indices[27] = 16;
    model.indices[28] = 18;
    model.indices[29] = 17;
    model.indices[30] = 20;
    model.indices[31] = 23;
    model.indices[32] = 22;
    model.indices[33] = 20;
    model.indices[34] = 22;
    model.indices[35] = 21;
    
    model.pos = pos;
    model.rot = glm::vec3(0.0f);
    model.scl = glm::vec3(1.0f);
    model.bounds.min = glm::vec3(0.0f);
    model.bounds.max = ext;
    model.ubo.model = glm::translate(glm::mat4(1.0f), pos);
    model.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;
    
    return model;
}

bool RayIntersectAxisAlignedBox(Ray ray, AxisAlignedBoundingBox box, glm::vec3 *intersection)
{
    // general method from https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans
    float t1 = (box.min.x - ray.origin.x) * ray.inverse_direction.x;
    float t2 = (box.max.x - ray.origin.x) * ray.inverse_direction.x;
    float t3 = (box.min.y - ray.origin.y) * ray.inverse_direction.y;
    float t4 = (box.max.y - ray.origin.y) * ray.inverse_direction.y;
    float t5 = (box.min.z - ray.origin.z) * ray.inverse_direction.z;
    float t6 = (box.max.z - ray.origin.z) * ray.inverse_direction.z;
    float t_min = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float t_max = t_max = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));
    float t = (t_min >= 0.0f) ? t_min : t_max;
    *intersection = ray.origin + glm::vec3(ray.direction.x * t, ray.direction.y * t, ray.direction.z * t);
    return t_max > fmax(t_min, 0.0f);
}

// TODO(Matt): Refactor - this is a prototype.
bool RaycastAgainstModelBounds(Ray ray, const Model *model, glm::vec3 *intersection)
{
    glm::mat4 inverse = glm::inverse(model->ubo.model);
    Ray local_ray = CreateRay(inverse * glm::vec4(ray.origin, 1.0f), inverse * glm::vec4(ray.direction, 0.0f), ray.length);
    bool intersect = RayIntersectAxisAlignedBox(local_ray, model->bounds, intersection);
    *intersection = model->ubo.model * glm::vec4(*intersection, 1.0f);
    return intersect;
}

Ray ScreenPositionToWorldRay(int32_t mouse_x, int32_t mouse_y, uint32_t screen_width, uint32_t screen_height, glm::mat4 view, glm::mat4 projection, float ray_length)
{
    glm::vec4 screen_start(((float)mouse_x / (float)screen_width  - 0.5f) * 2.0f, ((float)mouse_y / (float)screen_height - 0.5f) * 2.0f, -1.0, 1.0f);
    glm::vec4 screen_end(((float)mouse_x / (float)screen_width  - 0.5f) * 2.0f, ((float)mouse_y / (float)screen_height - 0.5f) * 2.0f, 0.0, 1.0f);
    glm::mat4 inverse_projection = glm::inverse(projection);
    glm::mat4 inverse_view = glm::inverse(view);
    glm::vec4 camera_start = inverse_projection * screen_start;
    camera_start /= camera_start.w;
    glm::vec4 world_start = inverse_view * camera_start;
    world_start /= world_start.w;
    glm::vec4 camera_end = inverse_projection * screen_end;
    camera_end /= camera_end.w;
    glm::vec4 world_end = inverse_view * camera_end;
    world_end /= world_end.w;
    glm::vec3 world_dir(world_end - world_start);
    return CreateRay(world_start, world_dir, ray_length);
}

Ray CreateRay(glm::vec3 origin, glm::vec3 direction, float length)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = glm::normalize(direction);
    ray.inverse_direction = 1.0f / direction;
    ray.length = length;
    return ray;
}

Model CreateDebugQuad2D(glm::vec2 pos, glm::vec2 ext, uint32_t material_type, uint32_t shader_id, glm::vec4 color, bool filled)
{
    Model model = {};
    model.material_type = material_type;
    model.shader_id = shader_id;
    model.uniform_count = GetUniformCount();
    model.vertex_count = 4;
    model.index_count = 6;
    model.vertices = (Vertex *)malloc(sizeof(Vertex) * model.vertex_count);
    model.indices = (uint32_t *)malloc(sizeof(uint32_t) * model.index_count);
    
    pos = (pos - 0.5f) * 2.0f;
    ext *= 2.0f;
    model.vertices[0].position = glm::vec3(pos.x, pos.y, 0.0f);
    model.vertices[1].position = glm::vec3(pos.x + ext.x, pos.y, 0.0f);
    model.vertices[2].position = glm::vec3(pos.x + ext.x, pos.y + ext.y, 0.0f);
    model.vertices[3].position = glm::vec3(pos.x, pos.y + ext.y, 0.0f);
    
    model.vertices[0].color = color;
    model.vertices[1].color = color;
    model.vertices[2].color = color;
    model.vertices[3].color = color;
    
    model.vertices[0].uv0 = glm::vec2(0.0f, 0.0f);
    model.vertices[1].uv0 = glm::vec2(1.0f, 0.0f);
    model.vertices[2].uv0 = glm::vec2(1.0f, 1.0f);
    model.vertices[3].uv0 = glm::vec2(0.0f, 1.0f);
    
    model.indices[0] = 0;
    model.indices[1] = 2;
    model.indices[2] = 1;
    model.indices[3] = 0;
    model.indices[4] = 3;
    model.indices[5] = 2;
    
    model.pos = {0.0f, 0.0f, 0.0f};
    model.rot = glm::vec3(0.0f);
    model.scl = glm::vec3(1.0f);
    model.hit_test_enabled = false;
    model.bounds = {};
    model.ubo.model = glm::mat4(1.0f);
    model.ubo.view_position = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;
    
    return model;
}