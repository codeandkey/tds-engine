#include "overlay.h"
#include "log.h"
#include "memory.h"
#include "engine.h"

#include <pango/pangocairo.h>
#include <stdlib.h>
#include <string.h>

static void _tds_overlay_get_coords(struct tds_overlay* ptr, float x, float y, float* _x, float* _y, int flags);
static void _tds_overlay_get_text_size(PangoLayout* layout, int* width, int* height);

struct tds_overlay* tds_overlay_create(int width, int height) {
	struct tds_overlay* output = tds_malloc(sizeof *output);

	output->width = width;
	output->height = height;

	output->surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	output->ctx = cairo_create(output->surf);

	glGenTextures(1, &output->gl_texture);
	glGenTextures(1, &output->gl_texture_backbuffer);

	glBindTexture(GL_TEXTURE_2D, output->gl_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, output->width, output->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, output->gl_texture_backbuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, output->width, output->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	return output;
}

void tds_overlay_free(struct tds_overlay* ptr) {
	glDeleteTextures(1, &ptr->gl_texture);
	glDeleteTextures(1, &ptr->gl_texture_backbuffer);

	cairo_surface_destroy(ptr->surf);
	cairo_destroy(ptr->ctx);

	tds_free(ptr);
}

unsigned int tds_overlay_update_texture(struct tds_overlay* ptr) {
	cairo_surface_flush(ptr->surf);

	unsigned char* img_data = cairo_image_surface_get_data(ptr->surf);

	glBindTexture(GL_TEXTURE_2D, ptr->gl_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ptr->width, ptr->height, GL_RGBA, GL_UNSIGNED_BYTE, img_data);

	unsigned int tmp_texture = ptr->gl_texture_backbuffer; /* Perform a texture swap. The overlay will be a frame behind but the texture upload won't be stalling the render process. */
	ptr->gl_texture_backbuffer = ptr->gl_texture;
	ptr->gl_texture = tmp_texture;

	return ptr->gl_texture;
}

void tds_overlay_set_color(struct tds_overlay* ptr, float r, float g, float b, float a) {
	cairo_set_source_rgba(ptr->ctx, r, g, b, a);
}

void tds_overlay_clear(struct tds_overlay* ptr) {
	cairo_save(ptr->ctx);
	cairo_set_operator(ptr->ctx, CAIRO_OPERATOR_SOURCE);
	cairo_paint(ptr->ctx);
	cairo_restore(ptr->ctx);
}

void tds_overlay_render_quad(struct tds_overlay* ptr, float l, float r, float t, float b, int flags) {
	_tds_overlay_get_coords(ptr, l, t, &l, &t, flags);
	_tds_overlay_get_coords(ptr, r, b, &r, &b, flags);

	cairo_rectangle(ptr->ctx, l, t, (r - l), (b - t));

	if (flags & TDS_OVERLAY_OUTLINE) {
		cairo_stroke(ptr->ctx);
	} else {
		cairo_fill(ptr->ctx);
	}
}

void tds_overlay_render_text(struct tds_overlay* ptr, float l, float r, float t, float b, float size, const char* buffer, int buffer_len, int flags) {
	_tds_overlay_get_coords(ptr, l, t, &l, &t, flags);
	_tds_overlay_get_coords(ptr, r, b, &r, &b, flags);

	PangoLayout* layout = NULL;
	PangoFontDescription* desc = NULL;

	layout = pango_cairo_create_layout(ptr->ctx);
	pango_layout_set_text(layout, buffer, buffer_len);

	desc = pango_font_description_from_string(TDS_OVERLAY_DEFAULT_FONT);
	pango_font_description_set_size(desc, size * PANGO_SCALE);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	int width, height;
	_tds_overlay_get_text_size(layout, &width, &height);

	if (flags & TDS_OVERLAY_HRIGHT) {
		l = r - width;
	} else if (flags & TDS_OVERLAY_HCENTER) {
		l = (r + l) / 2.0f - width / 2.0f;
	}

	if (flags & TDS_OVERLAY_VBOTTOM) {
		t = b - height;
	} else if (flags & TDS_OVERLAY_VCENTER) {
		t = (t + b) / 2.0f - height / 2.0f;
	}

	cairo_move_to(ptr->ctx, l, t);
	pango_cairo_show_layout(ptr->ctx, layout);
	g_object_unref(layout);
}

void _tds_overlay_get_coords(struct tds_overlay* ptr, float x, float y, float* _x, float* _y, int flags) {
	struct tds_camera* g_camera = tds_engine_global->camera_handle;

	float camera_left = g_camera->x  - g_camera->width / 2.0f;
	float camera_right = camera_left + g_camera->width;
	float camera_bottom = g_camera->y - g_camera->height / 2.0f;
	float camera_top = camera_bottom + g_camera->height;

	if (flags & TDS_OVERLAY_WORLDSPACE) {
		if (_x) *_x = ((x - camera_left) / (camera_right - camera_left)) * ptr->width;
		if (_y) *_y = (1.0f - ((y - camera_bottom) / (camera_top - camera_bottom))) * ptr->height;
	} else if (flags & TDS_OVERLAY_REL_SCREENSPACE) {
		if (_x) *_x = (x + 1.0f) / 2.0f * ptr->width;
		if (_y) *_y = (1.0f - (y + 1.0f) / 2.0f) * ptr->height;
	} else {
		if (_x) *_x = x;
		if (_y) *_y = y;
	}
}

void _tds_overlay_get_text_size(PangoLayout* layout, int* width, int* height) {
	pango_layout_get_size(layout, width, height);

	*width /= PANGO_SCALE;
	*height /= PANGO_SCALE;
}
