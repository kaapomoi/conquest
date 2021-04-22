#version 440

in vec2 fragment_position;
in vec4 fragment_color;
in vec2 fragment_uv;

out vec4 color;

uniform sampler2D k2d_texture;

void main(){

	vec4 texture_color = texture(k2d_texture, fragment_uv);
	
	color = fragment_color * texture_color;
}