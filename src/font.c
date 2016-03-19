#include "font.h"
#include "memory.h"
#include "log.h"

#include <GLXW/glxw.h>

struct tds_font* tds_font_create(struct tds_ft* ft, const char* filename, int size) {
	struct tds_font* output = tds_malloc(sizeof *output);

	int error = FT_New_Face(ft->ctx, filename, 0, &output->face);

	if (error == FT_Err_Unknown_File_Format) {
		tds_logf(TDS_LOG_WARNING, "Failed to load font %s: invalid file format\n", filename);
		tds_free(output);
		return NULL;
	} else if (error) {
		tds_logf(TDS_LOG_WARNING, "Failed to laod font %s: other error\n", filename);
		tds_free(output);
		return NULL;
	}

	output->size_px = size;
	FT_Set_Pixel_Sizes(output->face, 0, size);

	glGenTextures(sizeof output->glyph_textures / sizeof *(output->glyph_textures), output->glyph_textures);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (int i = 0; i < sizeof output->glyph_textures / sizeof *(output->glyph_textures); ++i) {
		glBindTexture(GL_TEXTURE_2D, output->glyph_textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (FT_Load_Char(output->face, i, FT_LOAD_RENDER)) {
			continue;
		}

		FT_GlyphSlot g = output->face->glyph;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
	}

	return output;
}

void tds_font_free(struct tds_font* ptr) {
	if (!ptr) {
		return;
	}

	glDeleteTextures(sizeof ptr->glyph_textures / sizeof *(ptr->glyph_textures), ptr->glyph_textures);

	FT_Done_Face(ptr->face);
	tds_free(ptr);
}
