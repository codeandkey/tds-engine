#pragma once

/*
 * TDS abstraction over FT2 font faces.
 */

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ft.h"

struct tds_font {
	FT_Face face;
	int size_px;
	unsigned int glyph_textures[256];
};

struct tds_font* tds_font_create(struct tds_ft* ctx, const char* filename, int size);
void tds_font_free(struct tds_font* font);
