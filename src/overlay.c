#include "overlay.h"
#include "log.h"
#include "memory.h"
#include "engine.h"

static void _tds_overlay_get_coords(struct tds_overlay* ptr, float x, float y, float* _x, float* _y, int flags);

struct tds_overlay* tds_overlay_create(int width, int height) {
	struct tds_overlay* output = tds_malloc(sizeof *output);

	output->width = width;
	output->height = height;

	output->surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	output->ctx = cairo_create(output->surf);

	glGenTextures(1, &output->gl_texture);
	glBindTexture(GL_TEXTURE_2D, output->gl_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, output->width, output->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	return output;
}

void tds_overlay_free(struct tds_overlay* ptr) {
	glDeleteTextures(1, &ptr->gl_texture);

	cairo_surface_destroy(ptr->surf);
	cairo_destroy(ptr->ctx);

	tds_free(ptr);
}

unsigned int tds_overlay_update_texture(struct tds_overlay* ptr) {
	cairo_surface_flush(ptr->surf);

	glBindTexture(GL_TEXTURE_2D, ptr->gl_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ptr->width, ptr->height, GL_RGBA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(ptr->surf));

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
		if (_y) *_y = (y - 1.0f) / 2.0f * ptr->height;
	} else {
		if (_x) *_x = x;
		if (_y) *_y = y;
	}
}
