#include "render_flat.h"
#include "memory.h"
#include "camera.h"
#include "display.h"
#include "engine.h"
#include "log.h"
#include "vertex_buffer.h"

static void transform_coords(struct tds_render_flat* ptr, float x, float y, float* ox, float* oy);

struct tds_render_flat* tds_render_flat_create(void) {
	struct tds_render_flat* output = tds_malloc(sizeof *output);
	struct tds_display* global_display = tds_engine_global->display_handle;
	output->rt_backbuf = tds_rt_create(global_display->desc.width, global_display->desc.height);

	output->shader_passthrough = tds_shader_create(TDS_RENDER_FLAT_PASSTHROUGH_VS, NULL, TDS_RENDER_FLAT_PASSTHROUGH_FS);
	output->shader_text = tds_shader_create(TDS_RENDER_FLAT_PASSTHROUGH_VS, NULL, TDS_RENDER_FLAT_TEXT_FS);

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

void tds_render_flat_text(struct tds_render_flat* ptr, struct tds_font* font, char* buf, int buflen, float _x, float _y, tds_render_alignment align) {
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
		if (buf[i] == TDS_RENDER_FLAT_CONTROL_CHAR) {
			i++;

			if (i >= buflen) {
				tds_logf(TDS_LOG_WARNING, "Malformed control sequence in string [%.*s], terminating render procedure\n", buflen, buf);
				return;
			}

			switch(buf[i++]) {
			case TDS_RENDER_FLAT_CONTROL_CHAR_COLOR:
				if (i + 5 >= buflen) {
					tds_logf(TDS_LOG_WARNING, "Malformed color control sequence in string [%.*s], terminating render procedure\n", buflen, buf);
					return;
				}

				i += 6;
				break;
			case TDS_RENDER_FLAT_CONTROL_CHAR_WAVE:
				if (i + 5 >= buflen) {
					tds_logf(TDS_LOG_WARNING, "Malformed wave control sequence in string [%.*s], terminating render procedure\n", buflen, buf);
					return;
				}

				i += 6;
				break;
			case TDS_RENDER_FLAT_CONTROL_CHAR_SHAKE:
				if (i + 1 >= buflen) {
					tds_logf(TDS_LOG_WARNING, "Malformed shake control sequence in string [%.*s], terminating render procedure\n", buflen, buf);
					return;
				}

				i += 2;
				break;
			case TDS_RENDER_FLAT_CONTROL_CHAR_END:
				if (i >= buflen) {
					tds_logf(TDS_LOG_WARNING, "Malformed end control sequence in string [%.*s], terminating render procedure\n", buflen, buf);
					return;
				}

				i += 1;
				break;
			}
		}

		if (i >= buflen) { /* Can happen if a control sequence appears at the end of the string. */
			break;
		}

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

	for (int i = 0; i < buflen; ++i) {
		if (buf[i] == TDS_RENDER_FLAT_CONTROL_CHAR) {
			i++;

			/* The input has already been validated. We can proceed knowing that the buffer is sufficiently large. */
			char r[3] = {0}, g[3] = {0}, b[3] = {0}, a[3] = {0};

			switch(buf[i++]) {
			case TDS_RENDER_FLAT_CONTROL_CHAR_COLOR:
				r[0] = buf[i];
				r[1] = buf[i + 1];
				g[0] = buf[i + 2];
				g[1] = buf[i + 3];
				b[0] = buf[i + 4];
				b[1] = buf[i + 5];

				char* endptr = NULL;

				long rv = strtol(r, &endptr, 16);

				if (endptr == r) {
					tds_logf(TDS_LOG_WARNING, "Invalid hex digits in red field of color control string [%.*s], stopping\n", buflen, buf);
					return;
				}

				long gv = strtol(g, &endptr, 16);

				if (endptr == g) {
					tds_logf(TDS_LOG_WARNING, "Invalid hex digits in green field of color control string [%.*s], stopping\n", buflen, buf);
					return;
				}

				long bv = strtol(b, &endptr, 16);

				if (endptr == b) {
					tds_logf(TDS_LOG_WARNING, "Invalid hex digits in blue field of color control string [%.*s], stopping\n", buflen, buf);
					return;
				}

				tds_shader_set_color(ptr->shader_text, (float) rv / 255.0f, (float) gv / 255.0f, (float) bv / 255.0f, ptr->a);

				i += 6;
				break;
			case TDS_RENDER_FLAT_CONTROL_CHAR_WAVE:
				i += 6;
				break;
			case TDS_RENDER_FLAT_CONTROL_CHAR_SHAKE:
				i += 2;
				break;
			case TDS_RENDER_FLAT_CONTROL_CHAR_END:
				i += 1;
				break;
			}
		}

		if (i >= buflen) { /* Can happen if a control sequence appears at the end of the string. */
			break;
		}

		if (FT_Load_Char(font->face, buf[i], FT_LOAD_RENDER)) {
			tds_logf(TDS_LOG_WARNING, "Failed to load font glpyh for character [%c (%d)]\n", buf[i], buf[i]);
			continue;
		}

		FT_GlyphSlot g = font->face->glyph;

		float xl = x + g->bitmap_left * sx, yt = y + g->bitmap_top * sy;
		float w = g->bitmap.width * sx, h = g->bitmap.rows * sy;

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

	switch (ptr->mode) {
	case TDS_RENDER_COORD_WORLDSPACE:
		*ox = ((x - (cam->x - cam->width / 2.0f)) / cam->width) * 2.0f - 1.0f;
		*oy = ((y - (cam->y - cam->height / 2.0f)) / cam->height) * 2.0f - 1.0f;
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