
#ifndef RENDERTYPES_H

// Stores vulkan instance/device information not dependent on swapchain.
struct VulkanInfo
{
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkDebugUtilsMessengerEXT debug_messenger;
    u32 graphics_index;
    u32 present_index;
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

struct DirectionalLight
{
    glm::vec4 direction;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 ambient;
};

struct PerDrawUniformObject
{
    glm::mat4 model;
};

struct PerFrameUniformObject
{
    glm::vec4 view_position;
    glm::mat4 view;
    glm::mat4 projection;
};

struct PerPassUniformObject
{
    // NOTE(Matt): This doesn't necessarily belong here, I just wanted to
    // test per-pass uniform support.
    DirectionalLight sun;
};

struct UniformBuffer
{
    char *buffer;
    u32 buffer_size;
    u32 per_pass_offset;
    u32 per_draw_offset;
    u32 object_count;
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

struct ModelData {
    void*    memory_block;
    size_t   memory_block_size;
    
    u32* indices;
    glm::vec3* position; // 12
    glm::vec3* normal;   // 24
    glm::vec4* tangent;  // 40
    glm::vec4* color;    // 56
    glm::vec2* uv0;      // 64
    glm::vec2* uv1;      // 72
    glm::vec2* uv2;      // 80
    
    size_t attribute_offsets[7];
};

struct Model_Separate_Data
{
    u32 vertex_count;
    u32 index_count;
    u32 uniform_index;
    AxisAlignedBoundingBox bounds;
    bool hit_test_enabled;
    
    u32 material_type;
    u32 shader_id;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;
    
    // Heap allocated:
    ModelData* model_data; // separate data format 
};

/*
struct Model
{
    u32 vertex_count;
    u32 index_count;
    UniformBufferObject ubo;
    AxisAlignedBoundingBox bounds;
    bool hit_test_enabled;
    
    u32 material_type;
    u32 shader_id;
    VkBuffer vertex_buffer;
    VkBuffer index_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkDeviceMemory index_buffer_memory;
    
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scl;
    
    // Heap allocated:
    Vertex *vertices;
    u32 *indices;
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    VkDescriptorSet *descriptor_sets;
    u32 uniform_count;
};

// Destroys a model, freeing associated resources.
void DestroyModel(Model *model, const VulkanInfo *vulkan_info);

// Creates a box given a world space position, size, and material info.
Model CreateBox(glm::vec3 pos, glm::vec3 ext, u32 material_type, u32 shader_id);
*/

// Creates a ray with a given world origin, direction, and length.
Ray CreateRay(glm::vec3 origin, glm::vec3 direction, float length);

// Tests a ray against a model's oriented bounding box, by transforming the ray into the model's local space and
// then performing an axis-aligned bounding box test. 
// Returns true if there was an intersection, false otherwise.
// Fills the intersection parameter with the intersection point if there was one.
bool RaycastAgainstModelBounds(const Ray ray, const Model_Separate_Data *model, glm::vec3 *intersection);

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
Ray ScreenPositionToWorldRay(s32 x, s32 y, u32 screen_width, u32 screen_height, glm::mat4 view, glm::mat4 proj, float ray_distance);

/*
// Creates a screen-space quad given a normalized (0-1 range where (0, 0) is
// the upper left) position and extent. Takes material type and shader ID
// to identify the material to use. Color is used by the debug material.
// If filled, creates an index buffer for triangle drawing. Otherwise,
// index buffer is for line drawing (outline bounds).
Model CreateDebugQuad2D(glm::vec2 pos, glm::vec2 ext, u32 material_type, u32 shader_id, glm::vec4 color, bool filled);
*/

#define RENDERTYPES_H
#endif