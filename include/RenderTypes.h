
#include "VulkanLoader.h"

#define GLM_FORCE_RADIANS
#pragma warning(push, 0)
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/norm.hpp"
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


struct AxisAlignedBoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 inverse_direction;
    float length;
};

struct Model
{
    uint32_t vertex_count;
    uint32_t index_count;
    UniformBufferObject ubo;
    AxisAlignedBoundingBox bounds;
    uint32_t shader_id;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    bool hit_test_enabled;
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;
    // Heap allocated:
    Vertex *vertices;
    uint32_t *indices;
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    VkDescriptorSetLayout *descriptor_set_layouts;
    VkDescriptorSet *descriptor_sets;
};

void DestroyModel(Model *model);

Model CreateBox(glm::vec3 pos, glm::vec3 ext, uint32_t shader_id);
Ray CreateRay(glm::vec3 origin, glm::vec3 direction, float length);

// Tests a ray against a model's oriented bounding box, by transforming the ray into the model's local space and
// then performing an axis-aligned bounding box test. 
// Returns true if there was an intersection, false otherwise.
// Fills the intersection parameter with the intersection point if there was one.
bool RaycastAgainstModelBounds(const Ray ray, const Model *model, glm::vec3 *intersection);

// Tests a ray against an axis-aligned bounding box, assuming both are in the same coordinate space.
// For each axis, tests the ray against both planes of the box. If the minimum distance in one axis exceeds the
// maximum for another, no intersection occurs.
// Returns true if there was an intersection, false otherwise.
// Fills the distance parameter with the intersection point if there was one.
bool RayIntersectAxisAlignedBox(const Ray *ray, const AxisAlignedBoundingBox *box, glm::vec3 *intersection);
Ray ScreenPositionToWorldRay(int32_t mouse_x, int32_t mouse_y, uint32_t screen_width, uint32_t screen_height, glm::mat4 view, glm::mat4 proj, float ray_distance);