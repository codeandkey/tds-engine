#version 150

in vec2 p_texcoord;
out vec4 o_color;

uniform sampler2D tds_texture;
uniform vec4 tds_color;

void main(void) {
	o_color = texture(tds_texture, p_texcoord) * tds_color;
}
