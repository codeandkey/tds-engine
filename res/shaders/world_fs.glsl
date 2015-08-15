#version 120

varying vec2 p_texcoord;

uniform sampler2D tds_texture;
uniform vec4 tds_color;

void main(void) {
	gl_FragColor = texture2D(tds_texture, p_texcoord) * tds_color;
}
