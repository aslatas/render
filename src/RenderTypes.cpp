
#include "RenderTypes.h"

// TODO(Matt): We aren't yet freeing vulkan buffers here.
void DestroyModel(Model *model)
{
    free(model->vertices);
    free(model->indices);
    free(model);
    model = nullptr;
}

Model CreateBox(glm::vec3 pos, glm::vec3 ext, uint32_t shader_id)
{
    Model model;
    model.shader_id = shader_id;
    model.vertex_count = 24;
    model.index_count = 36;
    model.vertices = (Vertex *)malloc(model.vertex_count * sizeof(Vertex));
    model.indices = (uint32_t *)malloc(sizeof(uint32_t) * model.index_count);
    model.vertices[ 0].position = pos + glm::vec3(0.0f, 0.0f, 0.0f);
    model.vertices[ 1].position = pos + glm::vec3(0.0f, 0.0f, ext.z);
    model.vertices[ 2].position = pos + glm::vec3(ext.x, 0.0f, ext.z);
    model.vertices[ 3].position = pos + glm::vec3(ext.x, 0.0f, 0.0f);
    model.vertices[ 4].position = pos + glm::vec3(ext.x, 0.0f, 0.0f);
    model.vertices[ 5].position = pos + glm::vec3(ext.x, 0.0f, ext.z);
    model.vertices[ 6].position = pos + glm::vec3(ext.x, ext.y, ext.z);
    model.vertices[ 7].position = pos + glm::vec3(ext.x, ext.y, 0.0f);
    model.vertices[ 8].position = pos + glm::vec3(ext.x, ext.y, 0.0f);
    model.vertices[ 9].position = pos + glm::vec3(ext.x, ext.y, ext.z);
    model.vertices[10].position = pos + glm::vec3(0.0f, ext.y, ext.z);
    model.vertices[11].position = pos + glm::vec3(0.0f, ext.y, 0.0f);
    model.vertices[12].position = pos + glm::vec3(0.0f, ext.y, 0.0f);
    model.vertices[13].position = pos + glm::vec3(0.0f, ext.y, ext.z);
    model.vertices[14].position = pos + glm::vec3(0.0f, 0.0f, ext.z);
    model.vertices[15].position = pos + glm::vec3(0.0f, 0.0f, 0.0f);
    model.vertices[16].position = pos + glm::vec3(0.0f, 0.0f, ext.z);
    model.vertices[17].position = pos + glm::vec3(0.0f, ext.y, ext.z);
    model.vertices[18].position = pos + glm::vec3(ext.x, ext.y, ext.z);
    model.vertices[19].position = pos + glm::vec3(ext.x, 0.0f, ext.z);
    model.vertices[20].position = pos + glm::vec3(0.0f, 0.0f, 0.0f);
    model.vertices[21].position = pos + glm::vec3(ext.x, 0.0f, 0.0f);
    model.vertices[22].position = pos + glm::vec3(ext.x, ext.y, 0.0f);
    model.vertices[23].position = pos + glm::vec3(0.0f, ext.y, 0.0f);
    
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
    
    model.ubo.model = glm::mat4();
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;
    
    return model;
}