#version 330 core

layout(location = 0) in vec3 pos; 

uniform mat4 shadowMatrix;

void main() {
	gl_Position = shadowMatrix * vec4(pos, 1);
}