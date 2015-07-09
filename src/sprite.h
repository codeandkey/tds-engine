#pragma once

#include "vertex_buffer.h"
#include "texture.h"
#include "linmath.h"

/* The sprite structure will interpret tilesets and prepare the GL textures.
 *
 * Sprites define an image and some traits about it such as an angle, offset, scale, size.
 *
 * It will also hold the VAO/VBO.
 * Textures are shared between sprites.
 * VBOs are NOT shared, due to size restrictions, etc.
 * Animations are not handled by sprites, since sprites are shared.
 * They are handled by the entity internally.
 */

struct tds_sprite {
	float r, g, b, a;
	float center_x, center_y;
	float offset_x, offset_y, offset_angle;
	float offset_scale;
	float width, height;
	float animation_rate; /* How many frames per second. */
	unsigned int current_frame;

	mat4x4 mat_transform, mat_id;

	struct tds_texture* texture;
	struct tds_vertex_buffer* vbo_handle;
};

struct tds_sprite* tds_sprite_create(struct tds_texture* texture, float width, float height);
void tds_sprite_free(struct tds_sprite* ptr);

/*
 * Encapsulating variable access into functions here will just be really slow and unnecessary.
 *
 * Changing the sprite should be done by manipulating the members.
 */

float* tds_sprite_get_transform(struct tds_sprite* ptr);
unsigned int tds_sprite_get_offset(struct tds_sprite* ptr);
