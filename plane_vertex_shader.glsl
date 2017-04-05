#version 330 core

layout(location = 0) in vec3 pos; 

out vec4 texCoor;


uniform mat4 cameraTransformation;
uniform mat4 perspective;
uniform mat4 shadowMatrix;
uniform mat4 view;

void main() {
	gl_Position = perspective * view * cameraTransformation * vec4(pos, 1);
	texCoor = shadowMatrix * vec4(pos,1);
}