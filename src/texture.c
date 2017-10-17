#include "texture.h"
#include "memory.h"
#include "log.h"
#include "stb_image.h"

#include <GLXW/glxw.h>
#include <string.h>

struct tds_texture* tds_texture_create(const char* filename, int tile_x, int tile_y) {
	struct tds_texture* output = tds_malloc(sizeof(struct tds_texture));

	int w = 0, h = 0;

	unsigned char* stb_data = stbi_load(filename, &w, &h, NULL, 4);

	if (!stb_data) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to load texture [%s]\n", filename);
		return NULL;
	}

	/*
	 * stb_image doesn't seem to be cooperative towards LD when it comes to linking stbi_set_flip_vertically_on_load; we will manually cycle the rows
	 */

	output->dim.x = w;
	output->dim.y = h;

	unsigned char* img_data = tds_malloc(sizeof *img_data * w * h * 4);
	unsigned int img_rw = 4 * w;

	for (int i = 0; i < h; ++i) {
		memcpy(img_data + i * img_rw, stb_data + (h - 1) * img_rw - i * img_rw, w * 4 * sizeof *img_data);
	}

	glGenTextures(1, &output->gl_id);
	glBindTexture(GL_TEXTURE_2D, output->gl_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	stbi_image_free(stb_data);
	tds_free(img_data);

	tds_logf(TDS_LOG_DEBUG, "Loaded texture %s : width %d, height %d\n", filename, w, h);

	if (tile_x < 0) {
		tile_x = w;
	}

	if (tile_y < 0) {
		tile_y = h;
	}

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
			target_frame->right = (float) (x + 1) / (float) tile_count_w; // - pixel_dist_x; /* We don't want to share texcoords with the next frame. */
			target_frame->top = (float) (y + 1) / (float) tile_count_h;
			target_frame->bottom = (float) y / (float) tile_count_h; // + pixel_dist_y; /* Another measure to avoid frame oversampling */
		}
	}

	glBindTexture(GL_TEXTURE_2D, output->gl_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	output->filename = tds_malloc(strlen(filename) + 1);
	memcpy(output->filename, filename, strlen(filename));
	output->filename[strlen(filename)] = 0;

	return output;
}

void tds_texture_set_wrap(struct tds_texture* ptr, int wrap_x, int wrap_y) {
	tds_logf(TDS_LOG_DEBUG, "Setting texture to wrap mode (%d, %d)\n", wrap_x, wrap_y);

	glBindTexture(GL_TEXTURE_2D, ptr->gl_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_x ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_y ? GL_REPEAT : GL_CLAMP_TO_EDGE);
}

void tds_texture_free(struct tds_texture* ptr) {
	glDeleteTextures(1, &ptr->gl_id);

	if (ptr->filename) {
		tds_free(ptr->filename);
	}

	tds_free(ptr->frame_list);
	tds_free(ptr);
}
