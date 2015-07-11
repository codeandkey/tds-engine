#version 150

in vec3 v_position;
in vec2 v_texcoord;

out vec2 p_texcoord;

void main(void) {
	gl_Position = vec4(v_position, 1.0);
	p_texcoord = v_texcoord;
}
