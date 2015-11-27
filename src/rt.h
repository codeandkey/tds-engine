#pragma once

/*
 * The TDS RT (render texture) is used heavily with graphical effects such as shadowing.
 */

struct tds_rt {
	unsigned int gl_fb, gl_rb, gl_tex;
	unsigned int width, height;
};

struct tds_rt* tds_rt_create(unsigned int width, unsigned int height);
void tds_rt_free(struct tds_rt* ptr);

void tds_rt_bind(struct tds_rt* ptr); /* This sets the output RT. If ptr is NULL, the RT is bound back to framebuffer 0 (the screen) */
