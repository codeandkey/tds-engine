#version 120

attribute vec3 v_position;
attribute vec2 v_texcoord;

varying vec2 p_texcoord;
uniform mat4 tds_transform;

void main(void) {
	gl_Position = tds_transform * vec4(v_position, 1.0);
	p_texcoord = v_texcoord;
}
