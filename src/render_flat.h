#pragma once

/* the render_flat subsystem is simply a (semi) modular abstraction over 2D rendering (lines, quads, text)
 * it can operate in WORLDSPACE, SCREENSPACE, OR REL_SCREENSPACE. This pixel coord distinction is extremely important.
 */

#include "rt.h"
#include "font.h"
#include "stringdb.h"
#include "clock.h"

#define TDS_RENDER_FLAT_PASSTHROUGH_VS "res/shaders/world_vs.glsl"
#define TDS_RENDER_FLAT_PASSTHROUGH_FS "res/shaders/color_fs.glsl"

#define TDS_RENDER_FLAT_TEXT_FS "res/shaders/text_fs.glsl"

#define TDS_RENDER_FLAT_PERIOD_MAX 2500 /* Millseconds to use as a period when PP = 0xFF. Apply a linear scale from this value to 0. */
#define TDS_RENDER_FLAT_SPEED_MAX  0.01f /* Factor to scale with the SS value and then multiply with the ms time. */

#define TDS_RENDER_RAND_PRECISION 1000

/* Control sequences in tds_render_flat text:
 *
 * Inserting special sequences into rendered text can allow for some interesting text behavior.
 * The control prefix is '^'.
 * The color control string is "cRRGGBB"
 * 	This sets the color for all future characters. The digits should be in hexadecimal, from 00 to FF.
 * The wave control string is "wIIPPSS"
 * To end a wave sequence, the control string is "ew"
 * 	The PP should be a byte indicating the sin period offset. More will increase the wave frequency.
 *	The II should be a byte indicating the amplitude of the wave.
 *	The SS should indicate the speed of oscillation. This is applied as a factor to the ms time.
 * The shake control string is "sII"
 * The shake termination string is "es"
 * 	The II indicates the intensity of the shake.
 */

typedef enum {
	TDS_RENDER_COORD_WORLDSPACE,
	TDS_RENDER_COORD_SCREENSPACE,
	TDS_RENDER_COORD_REL_SCREENSPACE
} tds_render_coord_mode;

typedef enum {
	TDS_RENDER_LALIGN,
	TDS_RENDER_RALIGN,
	TDS_RENDER_CALIGN
} tds_render_alignment;

struct tds_render_flat {
	struct tds_rt* rt_backbuf;
	struct tds_shader* shader_passthrough, *shader_text;
	tds_render_coord_mode mode;
	tds_clock_point cp_start;
	float r, g, b, a;
};

struct tds_render_flat* tds_render_flat_create(void);
void tds_render_flat_free(struct tds_render_flat* ptr);

void tds_render_flat_clear(struct tds_render_flat* ptr);
void tds_render_flat_set_mode(struct tds_render_flat* ptr, tds_render_coord_mode mode);
void tds_render_flat_set_color(struct tds_render_flat* ptr, float r, float g, float b, float a);

void tds_render_flat_line(struct tds_render_flat* ptr, float x1, float y1, float x2, float y2);
void tds_render_flat_point(struct tds_render_flat* ptr, float x, float y);
void tds_render_flat_text(struct tds_render_flat* ptr, struct tds_font* font, char* buf, int buflen, float l, float t, tds_render_alignment align, struct tds_string_format* formats);
