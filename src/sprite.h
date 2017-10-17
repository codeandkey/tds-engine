#pragma once

#include "vertex_buffer.h"
#include "texture.h"
#include "linmath.h"
#include "coord.h"

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

/*
 * to help with consistency sprites will be enforced to be exactly their adveritsed size in game units.
 * so, a 16x16 sprite will take exactly the space of one block in-game.
 * fortunately, this means sprites no longer need to keep track of their own sizes; they can refer to their source textures.
 *
 * vertex buffers will consider 1.0f x 1.0f to be one block in game space and will be scaled appropriately
 */

struct tds_sprite {
	float offset_angle, offset_scale;
	tds_vec2 offset;
	float animation_rate; /* Delay in ms */

	mat4x4 mat_transform, mat_id;

	struct tds_texture* texture;
	struct tds_vertex_buffer* vbo_handle;
};

struct tds_sprite* tds_sprite_create(struct tds_texture* texture, float animation_rate);
void tds_sprite_free(struct tds_sprite* ptr);

/*
 * Encapsulating variable access into functions here will just be really slow and unnecessary.
 *
 * Changing the sprite should be done by manipulating the members.
 */

vec4* tds_sprite_get_transform(struct tds_sprite* ptr);
