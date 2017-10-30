#include "sprite.h"
#include "log.h"
#include "memory.h"
#include "linmath.h"

#include <GLXW/glxw.h>
#include <string.h>

/* Inefficient as it may seem, we prepare a huge VBO with different texcoords for each texture frame. */
/* Rendering a gigantic VBO with variable offsets is way faster than switching textures each frame. */

struct tds_sprite* tds_sprite_create(struct tds_texture* texture, float animation_rate) {
	struct tds_sprite* output = tds_malloc(sizeof(struct tds_sprite));

	memset(output, 0, sizeof(struct tds_sprite));

	output->texture = texture;
	output->animation_rate = animation_rate;
	output->offset_angle = 0.0f;
	output->offset = tds_vec2_zero;

	struct tds_vertex* verts = tds_malloc(sizeof(struct tds_vertex) * texture->frame_count * 6);

	float right = (float) texture->dim.x / 16.0f; 
	float top = (float) texture->dim.y / 16.0f; 

	for (int i = 0; i < texture->frame_count; ++i) {
		struct tds_vertex tri[6] = {
			{0.0f, 0.0f, 0.0f, texture->frame_list[i].left, texture->frame_list[i].bottom},
			{right, 0.0f, 0.0f, texture->frame_list[i].right, texture->frame_list[i].bottom},
			{0.0f, top, 0.0f, texture->frame_list[i].left, texture->frame_list[i].top},
			{0.0f, top, 0.0f, texture->frame_list[i].left, texture->frame_list[i].top},
			{right, 0.0f, 0.0f, texture->frame_list[i].right, texture->frame_list[i].bottom},
			{right, top, 0.0f, texture->frame_list[i].right, texture->frame_list[i].top}
		};

		memcpy(verts + i * 6, tri, sizeof(struct tds_vertex) * 6);
	}

	output->vbo_handle = tds_vertex_buffer_create(verts, texture->frame_count * 6, GL_TRIANGLES);
	tds_free(verts);

	/* When rendering, use the vertex offset 6 * [frame ID, starting from 0] */

	return output;
}

void tds_sprite_free(struct tds_sprite* ptr) {
	tds_vertex_buffer_free(ptr->vbo_handle);
	tds_free(ptr);
}

vec4* tds_sprite_get_transform(struct tds_sprite* ptr) {
	/* Sprites have several transformations, including position, rotations, and scaling */

	mat4x4 pos, rot, scale;
	mat4x4 rotscale;

	mat4x4_identity(ptr->mat_transform);
	mat4x4_identity(ptr->mat_id);

	mat4x4_translate(pos, ptr->offset.x / 16.0f, ptr->offset.y / 16.0f, 0.0f);
	mat4x4_rotate_Z(rot, ptr->mat_id, ptr->offset_angle);
	mat4x4_scale(scale, ptr->mat_id, ptr->offset_scale);

	mat4x4_mul(rotscale, ptr->mat_transform, rot);
	mat4x4_mul(ptr->mat_transform, rotscale, pos);

	return ptr->mat_transform;
}
