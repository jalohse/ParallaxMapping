#version 330 core

layout(location = 0) in vec3 pos; 
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoordinate;

out vec4 texCoor;
out vec4 transformedNormal;
out vec4 viewVector;
out vec4 light;

uniform mat4 cameraTransformation;
uniform mat4 perspective;
uniform mat4 inverseCamera;
uniform mat4 lightRotation;
uniform mat4 view;
uniform mat4 shadowMatrix;
uniform vec3 lightPos;

void main() {
	gl_Position = perspective * view * cameraTransformation * vec4(pos, 1);
	viewVector = normalize(-gl_Position);
	light =  lightRotation * (vec4(lightPos, 1) - gl_Position);
	light = light + gl_Position;
	transformedNormal = inverseCamera * vec4(normal.x, normal.y, normal.z, 1);
	texCoor = shadowMatrix * vec4(pos,1);
}