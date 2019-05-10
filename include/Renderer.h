#ifndef RENDERER_H

#include "VulkanInit.h"

// TODO(Matt): We desperately need to decouple the renderer's idea of an object from the game's version.
// Currently, there's too much sharing going on - the renderer should operate on cached data, because
// all that it needs is material info, vertex/index/drawing info, and uniforms.

namespace Renderer
{
    // Basic API stuff.
    void Initialize();
    void Shutdown();
    void DrawFrame();
    void AddMaterial(MaterialCreateInfo *material_info, u32 material_type, VkRenderPass render_pass, u32 sub_pass);
    void OnWindowResized();
    
    // TODO(Matt): These should likely live elsewhere.
    void InitializeScene();
    void AddToScene(Model model);
    void DestroyScene();
    void UpdatePrePhysics(float delta);
    void UpdatePhysics(float delta);
    void UpdatePostPhysics(float delta);
    void UpdatePostRender(float delta);
    char* ReadShaderFile(const char* path, u32* length);
    void SelectObject(s32 mouse_x, s32 mouse_y, bool accumulate);
    void CreateModelBuffer(VkDeviceSize buffer_size, void* buffer_data, VkBuffer* buffer, VkDeviceMemory* buffer_memory, VkBufferUsageFlagBits flags);
    void UpdateTextureDescriptors(VkDescriptorSet descriptor_set);
    PerDrawUniformObject *GetPerDrawUniform(u32 object_index);
    PerFrameUniformObject *GetPerFrameUniform();
    PerPassUniformObject *GetPerPassUniform();
    void UpdateUniforms(u32 image_index);
    
    // TODO(Matt): These should probably be internal to the renderer component.
    void CreateGlobalUniformBuffers();
    void RecordCommands(u32 image_index);
    void CreateMaterials();
}
#define RENDERER_H
#endif

// Some reorganization thoughts:

// To facilitate minimal updates for the descriptor sets, I'm thinking the simple approach is to use a
// fixed block size of 16 or 32 uniforms or something, and keep an array of flags for blocks to update.
// Then we update any block that needs it - the other approach would be to try to update run lengths, but
// I imagine chunking it is the way to go.

// I'm not sure how to handle functions which are only needed by specific other components. For example,
// UpdatePre/Post/DuringPhysics is only called by the main loop. The main component needs access, but
// nobody else does.