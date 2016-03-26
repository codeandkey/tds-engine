#pragma once

/*
 * A structural implementation abstracting shaders (kinda)
 * render uses it's own nasty set of functions for loading most the game shaders (it was written previously)
 * TODO : transition render to use this structure for shader mgmt.
 */

struct tds_shader {
	const char* f_vs, *f_gs, *f_fs;
	unsigned int vs, gs, fs, prg;
	unsigned int u_transform, u_color;
};

struct tds_shader* tds_shader_create(const char* vs, const char* gs, const char* fs);
void tds_shader_free(struct tds_shader* ptr);

void tds_shader_bind(struct tds_shader* ptr);
void tds_shader_set_transform(struct tds_shader* ptr, float* transform);
void tds_shader_set_color(struct tds_shader* ptr, float r, float g, float b, float a);

void tds_shader_bind_texture(struct tds_shader* ptr, unsigned int texture);
void tds_shader_bind_texture_alt(struct tds_shader* ptr, unsigned int texture);
