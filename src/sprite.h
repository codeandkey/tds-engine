#pragma once

/* The sprite structure will interpret tilesets and prepare the GL textures.
 *
 * It will also hold the VAO/VBO.
 * Textures are shared between sprites.
 * VBOs are NOT shared, due to size restrictions, etc.
 */

struct tds_sprite {
	float r, g, b, a;
	float center_x, center_y;
	float offset_x, offset_y, offset_angle;
	float scale_x, scale_y;
	float width, height;

	struct tds_texture* texture;
	int current_frame;

	struct tds_vertex_buffer* vbo_handle;
};
