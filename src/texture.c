#include "texture.h"
#include "memory.h"
#include "log.h"

#include <SOIL/SOIL.h>
#include <GLXW/glxw.h>

struct tds_texture* tds_texture_create(const char* filename, int tile_x, int tile_y) {
	struct tds_texture* output = tds_malloc(sizeof(struct tds_texture));
	output->gl_id = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

	if (!output->gl_id) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to load texture %s\n", filename);
		return NULL;
	}

	glBindTexture(GL_TEXTURE_2D, output->gl_id);

	int w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	tds_logf(TDS_LOG_DEBUG, "Loaded texture %s : width %d, height %d\n", filename, w, h);

	float pixel_dist_x = 1.0f / (float) w, pixel_dist_y = 1.0f / (float) h;

	if (w % tile_x || h % tile_y) {
		tds_logf(TDS_LOG_WARNING, "Tile size does not divide evenly into image! (tile %dx%d, image %dx%d)\n", tile_x, tile_y, w, h);
	}

	int tile_count_w = (w / tile_x);
	int tile_count_h = (h / tile_y);

	tds_logf(TDS_LOG_DEBUG, "Tile count : %d by %d\n", tile_count_w, tile_count_h);

	output->frame_list = tds_malloc(sizeof(struct tds_texture) * tile_count_w * tile_count_h);
	output->frame_count = tile_count_w * tile_count_h;

	/* Earlier frames are in the top left, winding horizontally down. */

	for (int x = 0; x < tile_count_w; ++x) {
		for (int y = tile_count_h - 1; y >= 0; --y) {
			struct tds_texture_frame* target_frame = output->frame_list + x + (tile_count_h - (y + 1)) * tile_count_w;

			target_frame->left = (float) x / (float) tile_count_w;
			target_frame->right = (float) (x + 1) / (float) tile_count_w - pixel_dist_x; /* We don't want to share texcoords with the next frame. */
			target_frame->top = (float) (y + 1) / (float) tile_count_h;
			target_frame->bottom = (float) y / (float) tile_count_h + pixel_dist_y; /* Another measure to avoid frame oversampling */
		}
	}

	glBindTexture(GL_TEXTURE_2D, output->gl_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	output->filename = filename;

	return output;
}

void tds_texture_free(struct tds_texture* ptr) {
	glDeleteTextures(1, &ptr->gl_id);
	tds_free(ptr->frame_list);
	tds_free(ptr);
}
