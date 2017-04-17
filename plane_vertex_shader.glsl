#version 330 core

layout(location = 0) in vec3 pos; 
layout(location = 1) in vec3 tex;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;

out vec2 texCoor;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

uniform mat4 cameraTransformation;
uniform mat4 perspective;
uniform mat4 view;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform vec3 normal;

void main() {
	gl_Position = perspective * view * cameraTransformation * vec4(pos, 1);

	vec3 tangent = normalize(vec3(cameraTransformation * vec4(tangent, 0.0)));
	vec3 normal = normalize(vec3(cameraTransformation * vec4(normal, 0.0)));
	vec3 bitangent = normalize(vec3(cameraTransformation * vec4(bitangent, 0.0)));
	mat3 tangentBitangentNormal = transpose(mat3(tangent, bitangent, normal));
	tangentLightPos = tangentBitangentNormal * lightPos;
	tangentViewPos = tangentBitangentNormal * cameraPos;
	tangentFragPos = tangentBitangentNormal * vec3(cameraTransformation * vec4(pos, 0.0));

	texCoor = tex.xy;
}