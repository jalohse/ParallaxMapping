#version 330 core

uniform samplerCube tex;

out vec4 color;
in vec3 texCoor;


void main() {
	color =  texture(tex, texCoor);
}