#pragma once
#include "VulkanLoader.h"
#define GLM_FORCE_RADIANS
#pragma warning(push, 0)
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/norm.hpp"
#pragma warning(pop)
#include "stb/stb_ds.h"

// Stores vulkan instance/device information not dependent on swapchain.
struct VulkanInfo
{
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debug_messenger;
    uint32_t graphics_index;
    uint32_t present_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    // NOTE(Matt): Likely, graphics and present are the same queue. If so,
    // calls to Vulkan cannot treat them as separate, hence this flag.
    bool use_shared_queue;
    VkCommandPool primary_command_pool;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_set_layout;
    VkSampleCountFlagBits msaa_samples;
    VkFence *in_flight_fences;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
};

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

// Contain offsets/strides for attributes. Essential for creating the Attributes
// Offset
// Size
// Stride
struct BufferDataInfo {
    size_t index_offset = 0;
    size_t index_size = 0;
    size_t index_stride = 0;
    VkFormat index_format = VK_FORMAT_UNDEFINED;

    size_t pos_offset = 0;
    size_t pos_size = 0;
    size_t pos_stride = 0;
    VkFormat pos_format = VK_FORMAT_UNDEFINED;
    
    size_t normal_offset = 0;
    size_t normal_size = 0;
    size_t normal_stride = 0;
    VkFormat normal_format = VK_FORMAT_UNDEFINED;
    
    size_t tangent_offset = 0;
    size_t tangent_size = 0;
    size_t tangent_stride = 0;
    VkFormat tangent_format = VK_FORMAT_UNDEFINED;

    size_t tex_0_offset = 0;
    size_t tex_0_size = 0;
    size_t tex_0_stride = 0;
    VkFormat tex_0_format = VK_FORMAT_UNDEFINED;

    size_t tex_1_offset = 0;
    size_t tex_1_size = 0;
    size_t tex_1_stride = 0;
    VkFormat tex_1_format = VK_FORMAT_UNDEFINED;

    size_t color_0_offset = 0;
    size_t color_0_size = 0;
    size_t color_0_stride = 0;
    VkFormat color_0_format = VK_FORMAT_UNDEFINED;
};

struct ModelData {
    void*    memory_block;
    size_t   memory_block_size;

    uint32_t* indices;
    glm::vec3* position;
    glm::vec3* normal;   // 24
    glm::vec4* tangent;
    glm::vec4* color; // 40
    glm::vec2* uv0;   // 48
    glm::vec2* uv1;   // 56
    glm::vec2* uv2;   // 64
};

struct Model_Separate_Data
{
    BufferDataInfo attribute_data_info;

    uint32_t vertex_count;
    uint32_t index_count;
    UniformBufferObject ubo;
    AxisAlignedBoundingBox bounds;
    bool hit_test_enabled;
    
    uint32_t material_type;
    uint32_t shader_id;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;
    
    // Heap allocated:
    ModelData* model_data; // separate data format 
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    VkDescriptorSet *descriptor_sets;
    uint32_t uniform_count;
};

struct Model
{
    uint32_t vertex_count;
    uint32_t index_count;
    UniformBufferObject ubo;
    AxisAlignedBoundingBox bounds;
    bool hit_test_enabled;
    
    uint32_t material_type;
    uint32_t shader_id;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;
    
    // Heap allocated:
    Vertex *vertices;
    uint32_t *indices;
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    VkDescriptorSet *descriptor_sets;
    uint32_t uniform_count;
};

// struct Model
// {
//     uint32_t vertex_count;
//     uint32_t index_count;
//     UniformBufferObject ubo;
//     AxisAlignedBoundingBox bounds;
//     bool hit_test_enabled;
    
//     uint32_t material_type;
//     uint32_t shader_id;
//     VkBuffer vertex_buffer;
//     VkBuffer index_buffer;
//     VkDeviceMemory vertex_buffer_memory;
//     VkDeviceMemory index_buffer_memory;
    
//     glm::vec3 pos;
//     glm::vec3 rot;
//     glm::vec3 scl;
    
//     // Heap allocated:
//     Vertex *vertices;
//     uint32_t *indices;
//     VkBuffer *uniform_buffers;
//     VkDeviceMemory *uniform_buffers_memory;
//     VkDescriptorSet *descriptor_sets;
//     uint32_t uniform_count;
// };

// Destroys a model, freeing associated resources.
void DestroyModel(Model *model, const VulkanInfo *vulkan_info);
void DestroyModelSeparateDataTest(Model_Separate_Data *model, const VulkanInfo *vulkan_info);

// Creates a box given a world space position, size, and material info.
Model CreateBox(glm::vec3 pos, glm::vec3 ext, uint32_t material_type, uint32_t shader_id);
Model_Separate_Data CreateBoxNonInterleaved(glm::vec3 pos, glm::vec3 ext, uint32_t material_type, uint32_t shader_id);


// Creates a ray with a given world origin, direction, and length.
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

// Converts an integer screen position to a ray, with the origin at the
// world location of the screen position, and the direction towards
// the object under the screen position. Takes in screen coordinates (which
// can be negative if the position is outside the window), the screen size,
// and projection/view matrices to deproject.
// Returns a Ray object.
Ray ScreenPositionToWorldRay(int32_t x, int32_t y, uint32_t screen_width, uint32_t screen_height, glm::mat4 view, glm::mat4 proj, float ray_distance);

// Creates a screen-space quad given a normalized (0-1 range where (0, 0) is
// the upper left) position and extent. Takes material type and shader ID
// to identify the material to use. Color is used by the debug material.
// If filled, creates an index buffer for triangle drawing. Otherwise,
// index buffer is for line drawing (outline bounds).
Model CreateDebugQuad2D(glm::vec2 pos, glm::vec2 ext, uint32_t material_type, uint32_t shader_id, glm::vec4 color, bool filled);