#version 440 

in vec2 vertex_position;
in vec4 vertex_color;
in vec2 vertex_uv;

out vec4 fragment_color;
out vec2 fragment_position;
out vec2 fragment_uv;

uniform mat4 camera;

void main(){
	gl_Position.xy = (camera * vec4(vertex_position, 0.0, 1.0)).xy;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
	
	fragment_position = vertex_position;
	
	fragment_color = vertex_color;
	
	fragment_uv = vec2(vertex_uv.x, 1.0 - vertex_uv.y);
}