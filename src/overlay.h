#pragma once

#include "display.h"
#include <cairo/cairo.h>

#define TDS_OVERLAY_HLEFT    1
#define TDS_OVERLAY_HCENTER (1 << 2)
#define TDS_OVERLAY_HRIGHT  (1 << 3)
#define TDS_OVERLAY_VTOP    (1 << 4)
#define TDS_OVERLAY_VCENTER (1 << 5)
#define TDS_OVERLAY_VBOTTOM (1 << 6)
#define TDS_OVERLAY_OUTLINE (1 << 7)
#define TDS_OVERLAY_WORLDSPACE (1 << 8)
#define TDS_OVERLAY_SCREENSPACE (1 << 9)
#define TDS_OVERLAY_REL_SCREENSPACE (1 << 10)

#define TDS_OVERLAY_DEFAULT_FONT "DeJa Vu Sans Mono"

struct tds_overlay {
	cairo_t* ctx;
	cairo_surface_t* surf;

	int width, height;
	unsigned int gl_texture, gl_texture_backbuffer;
};

struct tds_overlay* tds_overlay_create(int width, int height);
void tds_overlay_free(struct tds_overlay* ptr);

void tds_overlay_set_color(struct tds_overlay* ptr, float r, float g, float b, float a);
void tds_overlay_clear(struct tds_overlay* ptr);

void tds_overlay_render_quad(struct tds_overlay* ptr, float l, float r, float t, float b, int flags);
void tds_overlay_render_text(struct tds_overlay* ptr, float l, float r, float t, float b, float size, const char* buffer, int buffer_len, int flags);
void tds_overlay_render_circle(struct tds_overlay* ptr, float x, float y, float r, int flags);
void tds_overlay_render_line(struct tds_overlay* ptr, float x1, float y1, float x2, float y2, float width, int flags);

unsigned int tds_overlay_update_texture(struct tds_overlay* ptr);
