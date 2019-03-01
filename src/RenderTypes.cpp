
#include "RenderTypes.h"

// TODO(Matt): We aren't yet freeing vulkan buffers here.
void DestroyModel(Model *model)
{
    free(model->vertices);
    free(model->indices);
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
    
    model.bounds.pos = pos;
    model.bounds.ext = ext;
    model.ubo.model = glm::mat4();
    model.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model.ubo.projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 10.0f);
    model.ubo.projection[1][1] *= -1;
    
    return model;
}

// TODO(Matt): Refactor - this is a prototype.
bool RaycastAgainstBoundingBox(glm::vec3 ray_origin, glm::vec3 ray_direction, float max_dist,float *hit_dist, Model *model)
{
	
	glm::vec3 bounds_min = model->bounds.pos;
    glm::vec3 bounds_max = model->bounds.ext - bounds_min;
    *hit_dist = -1.0f;
	float t_min = 0.0f;
	float t_max = max_dist;
    
	glm::vec3 world_pos = bounds_min;
    
	glm::vec3 delta = world_pos - ray_origin;
    
	{
		glm::vec3 x_axis(model->ubo.model[0].x, model->ubo.model[0].y, model->ubo.model[0].z);
		float e = glm::dot(x_axis, delta);
		float f = glm::dot(ray_direction, x_axis);
        
		if (fabs(f) > 0.0001f) {
			float t1 = (e + bounds_min.x) / f;
			float t2 = (e + bounds_max.x) / f;
			if (t1 > t2) {
				float w = t1;
                t1 = t2;
                t2 = w;
            }
			if (t2 < t_max) t_max = t2;
			if (t1 > t_min) t_min = t1;
			if (t_max < t_min) return false;
		} else {
			if (-e + bounds_min.x > 0.0f || -e + bounds_max.x < 0.0f) return false;
		}
	}
    
	{
		glm::vec3 y_axis(model->ubo.model[1].x, model->ubo.model[1].y, model->ubo.model[1].z);
		float e = glm::dot(y_axis, delta);
		float f = glm::dot(ray_direction, y_axis);
		if (fabs(f) > 0.0001f) {
			float t1 = (e + bounds_min.y) / f;
			float t2 = (e + bounds_max.y) / f;
			if (t1 > t2) {
                float w = t1;
                t1 = t2;
                t2 = w;
            }
			if (t2 < t_max) t_max = t2;
			if (t1 > t_min) t_min = t1;
			if (t_min > t_max) return false;
		} else {
			if (-e + bounds_min.y > 0.0f || -e + bounds_max.y < 0.0f) return false;
		}
	}
    
	{
		glm::vec3 z_axis(model->ubo.model[2].x, model->ubo.model[2].y, model->ubo.model[2].z);
		float e = glm::dot(z_axis, delta);
		float f = glm::dot(ray_direction, z_axis);
		if (fabs(f) > 0.0001f) {
			float t1 = (e + bounds_min.z) / f;
			float t2 = (e + bounds_max.z) / f;
			if (t1 > t2){
                float w = t1;
                t1 = t2;
                t2 = w;
            }
			if (t2 < t_max) t_max = t2;
			if (t1 > t_min) t_min = t1;
			if (t_min > t_max) return false;
		} else {
			if (-e + bounds_min.z > 0.0f || -e + bounds_max.z < 0.0f) return false;
		}
	}
    
    *hit_dist = t_min;
	return true;
}

void ScreenPositionToWorldRay(int32_t mouse_x, int32_t mouse_y, uint32_t screen_width, uint32_t screen_height, glm::mat4 view, glm::mat4 proj, glm::vec3 *out_pos, glm::vec3 *out_dir)
{
	glm::vec4 screen_start(((float)mouse_x / (float)screen_width  - 0.5f) * 2.0f, ((float)mouse_y / (float)screen_height - 0.5f) * 2.0f, -1.0, 1.0f);
	glm::vec4 screen_end(((float)mouse_x / (float)screen_width  - 0.5f) * 2.0f, ((float)mouse_y / (float)screen_height - 0.5f) * 2.0f, 0.0, 1.0f);
    
    
	glm::mat4 inverse_proj = glm::inverse(proj);
	
	glm::mat4 inverse_view = glm::inverse(view);
	
	glm::vec4 camera_start = inverse_proj * screen_start;    camera_start/=camera_start.w;
	glm::vec4 world_start  = inverse_view       * camera_start; world_start /=world_start .w;
	glm::vec4 camera_end   = inverse_proj * screen_end;      camera_end  /=camera_end  .w;
	glm::vec4 world_end    = inverse_view       * camera_end;   world_end   /=world_end   .w;
    
	glm::vec3 world_dir(world_end - world_start);
	world_dir = glm::normalize(world_dir);
    
	*out_pos = glm::vec3(world_start);
    *out_dir = glm::normalize(world_dir);
}