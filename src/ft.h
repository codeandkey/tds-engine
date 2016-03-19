#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

/* This structure exists because it has nowhere else to go. */

struct tds_ft {
	FT_Library ctx;
};

struct tds_ft* tds_ft_create(void);
void tds_ft_free(struct tds_ft* ptr);
