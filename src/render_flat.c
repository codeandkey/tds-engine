#include "render_flat.h"
#include "memory.h"
#include "camera.h"
#include "display.h"
#include "engine.h"
#include "log.h"
#include "vertex_buffer.h"

#include <math.h>

static void transform_coords(struct tds_render_flat* ptr, float x, float y, float* ox, float* oy);

struct tds_render_flat* tds_render_flat_create(void) {
	struct tds_render_flat* output = tds_malloc(sizeof *output);
	struct tds_display* global_display = tds_engine_global->display_handle;
	output->rt_backbuf = tds_rt_create(global_display->desc.width, global_display->desc.height);

	output->shader_passthrough = tds_shader_create(TDS_RENDER_FLAT_PASSTHROUGH_VS, NULL, TDS_RENDER_FLAT_PASSTHROUGH_FS);
	output->shader_text = tds_shader_create(TDS_RENDER_FLAT_PASSTHROUGH_VS, NULL, TDS_RENDER_FLAT_TEXT_FS);
	output->shader_color = tds_shader_create(TDS_RENDER_FLAT_PASSTHROUGH_VS, NULL, TDS_RENDER_FLAT_COLOR_FS);
	output->cp_start = tds_clock_get_point();

	tds_render_flat_set_mode(output, TDS_RENDER_COORD_REL_SCREENSPACE);
	tds_render_flat_set_color(output, 1.0f, 1.0f, 1.0f, 1.0f);

	glLineWidth(0.2f);
	glDisable(GL_LINE_SMOOTH);

	return output;
}

void tds_render_flat_free(struct tds_render_flat* ptr) {
	tds_rt_free(ptr->rt_backbuf);
	tds_shader_free(ptr->shader_passthrough);
	tds_shader_free(ptr->shader_text);
	tds_shader_free(ptr->shader_color);
	tds_free(ptr);
}

void tds_render_flat_clear(struct tds_render_flat* ptr) {
	tds_rt_bind(ptr->rt_backbuf);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void tds_render_flat_set_mode(struct tds_render_flat* ptr, tds_render_coord_mode mode) {
	ptr->mode = mode;
}

void tds_render_flat_set_color(struct tds_render_flat* ptr, float r, float g, float b, float a) {
	ptr->r = r;
	ptr->g = g;
	ptr->b = b;
	ptr->a = a;
}

void tds_render_flat_line(struct tds_render_flat* ptr, float x1, float y1, float x2, float y2) {
	struct tds_vertex verts[2] = {0};

	transform_coords(ptr, x1, y1, &verts[0].x, &verts[0].y);
	transform_coords(ptr, x2, y2, &verts[1].x, &verts[1].y);

	struct tds_vertex_buffer* vb = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_LINES);

	tds_rt_bind(ptr->rt_backbuf);
	tds_vertex_buffer_bind(vb);
	tds_shader_bind(ptr->shader_passthrough);

	mat4x4 ident;
	mat4x4_identity(ident);

	tds_shader_set_transform(ptr->shader_passthrough, (float*) *ident);
	tds_shader_set_color(ptr->shader_passthrough, ptr->r, ptr->g, ptr->b, ptr->a);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(vb->render_mode, 0, vb->vertex_count);

	tds_vertex_buffer_free(vb);
}

void tds_render_flat_quad(struct tds_render_flat* ptr, float left, float right, float top, float bottom, struct tds_texture* tex) {
	struct tds_vertex verts[6] = {
		{0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
	};

	transform_coords(ptr, left, top, &verts[0].x, &verts[0].y);
	transform_coords(ptr, right, bottom, &verts[1].x, &verts[1].y);
	transform_coords(ptr, right, top, &verts[2].x, &verts[2].y);
	transform_coords(ptr, left, top, &verts[3].x, &verts[3].y);
	transform_coords(ptr, right, bottom, &verts[4].x, &verts[4].y);
	transform_coords(ptr, left, bottom, &verts[5].x, &verts[5].y);

	struct tds_vertex_buffer* vb = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_TRIANGLES);

	tds_rt_bind(ptr->rt_backbuf);
	tds_vertex_buffer_bind(vb);

	struct tds_shader* target_shader = ptr->shader_color;
	tds_shader_bind(target_shader);

	if (tex) {
		target_shader = ptr->shader_passthrough;
		tds_shader_bind(target_shader);
		tds_shader_bind_texture(target_shader, tex->gl_id);
	}

	mat4x4 ident;
	mat4x4_identity(ident);

	tds_shader_set_transform(target_shader, (float*) *ident);
	tds_shader_set_color(target_shader, ptr->r, ptr->g, ptr->b, ptr->a);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(vb->render_mode, 0, vb->vertex_count);

	tds_vertex_buffer_free(vb);
}

void tds_render_flat_point(struct tds_render_flat* ptr, float x, float y) {
	struct tds_vertex verts[1] = {0};
	transform_coords(ptr, x, y, &verts[0].x, &verts[0].y);

	struct tds_vertex_buffer* vb = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_POINTS);

	tds_rt_bind(ptr->rt_backbuf);
	tds_vertex_buffer_bind(vb);
	tds_shader_bind(ptr->shader_passthrough);

	mat4x4 ident;
	mat4x4_identity(ident);

	tds_shader_set_transform(ptr->shader_passthrough, (float*) *ident);
	tds_shader_set_color(ptr->shader_passthrough, ptr->r, ptr->g, ptr->b, ptr->a);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(vb->render_mode, 0, vb->vertex_count);
}

void tds_render_flat_text(struct tds_render_flat* ptr, struct tds_font* font, char* buf, int buflen, float _x, float _y, tds_render_alignment align, struct tds_string_format* formats) {
	if (!font) {
		return;
	}

	struct tds_display* disp = tds_engine_global->display_handle;

	float total_width = 0.0f;
	float x, y;
	transform_coords(ptr, _x, _y, &x, &y);

	/* Scale factors to help with FreeType2's pixel coord system */
	float sx = 2.0f / (float) disp->desc.width, sy = 2.0f / (float) disp->desc.height;

	tds_rt_bind(ptr->rt_backbuf);
	tds_shader_bind(ptr->shader_text);

	mat4x4 ident;
	mat4x4_identity(ident);

	tds_shader_set_transform(ptr->shader_text, (float*) *ident);
	tds_shader_set_color(ptr->shader_text, ptr->r, ptr->g, ptr->b, ptr->a);

	for (int i = 0; i < buflen; ++i) {
		if (FT_Load_Char(font->face, buf[i], FT_LOAD_DEFAULT)) {
			tds_logf(TDS_LOG_WARNING, "Failed to load font glpyh for character [%c (%d)]\n", buf[i], buf[i]);
			continue;
		}

		FT_GlyphSlot g = font->face->glyph;

		total_width += g->bitmap_left * sx;
		total_width += g->bitmap.width * sx;
		total_width += (g->advance.x >> 6) * sx;
	}

	float x_offset = 0.0f;

	if (align == TDS_RENDER_CALIGN) {
		x_offset = -total_width / 2.0f;
	} else if (align == TDS_RENDER_RALIGN) {
		x_offset = total_width / 2.0f;
	}

	int in_wave = 0, in_shake = 0;

	float shake_offset_max = 0.0f;
	float wave_speed, wave_length, wave_amp;

	for (int i = 0; i < buflen; ++i) {
		struct tds_string_format* cur_format = formats;

		while (cur_format) {
			if (cur_format->pos == i) {
				switch (cur_format->type) {
				case TDS_STRING_FORMAT_TYPE_COLOR:
					tds_shader_set_color(ptr->shader_text, (float) cur_format->fields[0] / 255, (float) cur_format->fields[1] / 255, (float) cur_format->fields[2] / 255, ptr->a);
					break;
				case TDS_STRING_FORMAT_TYPE_SHAKE:
					in_shake = 1;
					shake_offset_max = ((float) cur_format->fields[0] / 255.0f);
					break;
				case TDS_STRING_FORMAT_TYPE_WAVE:
					in_wave = 1;
					wave_speed = ((float) cur_format->fields[0] / 255.0f);
					wave_length = ((float) cur_format->fields[1] / 255.0f);
					wave_amp = ((float) cur_format->fields[2] / 255.0f);
					break;
				case TDS_STRING_FORMAT_TYPE_END:
					in_wave = in_shake = 0;
					break;
				}
			}

			cur_format = cur_format->next;
		}

		if (FT_Load_Char(font->face, buf[i], FT_LOAD_RENDER)) {
			tds_logf(TDS_LOG_WARNING, "Failed to load font glpyh for character [%c (%d)]\n", buf[i], buf[i]);
			continue;
		}

		FT_GlyphSlot g = font->face->glyph;

		float xl = x + g->bitmap_left * sx, yt = y + g->bitmap_top * sy;
		float w = g->bitmap.width * sx, h = g->bitmap.rows * sy;

		if (in_shake) {
			yt += shake_offset_max * h * (((rand() % TDS_RENDER_RAND_PRECISION) / (float) TDS_RENDER_RAND_PRECISION) * 2.0f - 1.0f);
		}

		if (in_wave) {
			yt += sinf(tds_clock_get_ms(ptr->cp_start) * TDS_RENDER_FLAT_SPEED_MAX * wave_speed + TDS_RENDER_FLAT_PERIOD_MAX * wave_length * i) * h * wave_amp;
		}

		struct tds_vertex verts[4] = {
			{xl + x_offset, yt, 0.0f, 0.0f, 0.0f},
			{xl + x_offset + w, yt, 0.0f, 1.0f, 0.0f},
			{xl + x_offset, yt - h, 0.0f, 0.0f, 1.0f},
			{xl + x_offset + w, yt - h, 0.0f, 1.0f, 1.0f},
		};

		struct tds_vertex_buffer* vbo = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_TRIANGLE_STRIP);

		glBindTexture(GL_TEXTURE_2D, font->glyph_textures[(int) buf[i]]);
		glDrawArrays(vbo->render_mode, 0, vbo->vertex_count);

		tds_vertex_buffer_free(vbo);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}
}

void transform_coords(struct tds_render_flat* ptr, float x, float y, float* ox, float* oy) {
	struct tds_camera* cam = tds_engine_global->camera_handle;
	struct tds_display* disp = tds_engine_global->display_handle;

	float cam_width = cam->dim.x / 16.0f;
	float cam_left = cam->pos.x / 16.0f - cam_width / 2.0f;
	float cam_height = cam->dim.y / 16.0f;
	float cam_bottom = cam->pos.y / 16.0f - cam_height / 2.0f;

	switch (ptr->mode) {
	case TDS_RENDER_COORD_WORLDSPACE:
		*ox = (x - (cam_left / cam_width)) * 2.0f - 1.0f;
		*oy = (y - (cam_bottom / cam_height)) * 2.0f - 1.0f;

		break;
	case TDS_RENDER_COORD_SCREENSPACE:
		*ox = (x / disp->desc.width) * 2.0f - 1.0f;
		*oy = (1.0f - (y / disp->desc.height)) * 2.0f - 1.0f;
		break;
	case TDS_RENDER_COORD_REL_SCREENSPACE:
		*ox = x;
		*oy = y;
		break;
	}
}
