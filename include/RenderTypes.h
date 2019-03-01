
#include "VulkanLoader.h"

#define GLM_FORCE_RADIANS
#pragma warning(push, 0)
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#pragma warning(pop)
#pragma once

// TODO(Matt): Check out what other people are using here - maybe move up to
// 128 bytes?
// TODO(Matt): Extract vertex buffer, index buffer, and UBO into a generic
// object. Maybe include some material properties (probably in the UBO).
struct Vertex
{
    glm::vec3 position; // 12
    glm::vec3 normal;   // 24
    //glm::vec3 tangent;
    glm::vec4 color; // 40
    glm::vec2 uv0;   // 48
    glm::vec2 uv1;   // 56
    glm::vec2 uv2;   // 64
};

struct DirectionalLight
{
    glm::vec4 direction;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 ambient;
};

// TODO(Matt): Decide on a standard UBO for opaque objects, or choose a way to link objects with their UBOs.
struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 view_position;
    DirectionalLight sun;
};


struct BoundingBox
{
    glm::vec3 pos;
    glm::vec3 ext;
};

struct Model
{
    Vertex *vertices;
    uint32_t vertex_count;
    uint32_t *indices;
    uint32_t index_count;
    UniformBufferObject ubo;
    BoundingBox bounds;
    uint32_t shader_id;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    // Heap allocated:
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
};

void DestroyModel(Model *model);

Model CreateBox(glm::vec3 position, glm::vec3 extent, uint32_t shader_id);

bool RaycastAgainstBoundingBox(glm::vec3 ray_origin, glm::vec3 ray_direction, float max_dist,float *hit_dist, Model *model);

void ScreenPositionToWorldRay(int32_t mouse_x, int32_t mouse_y, uint32_t screen_width, uint32_t screen_height, glm::mat4 view, glm::mat4 proj, glm::vec3 *out_pos, glm::vec3 *out_dir);