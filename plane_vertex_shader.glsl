#version 330 core

layout(location = 0) in vec3 pos; 
layout(location = 1) in vec3 tex;
layout(location = 2) in vec3 tangent;

out mat3 tangentBitangentNormal;
out vec4 texCoor;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

uniform mat4 cameraTransformation;
uniform mat4 perspective;
uniform mat4 shadowMatrix;
uniform mat4 view;
uniform vec3 lightPos;
uniform vec3 cameraPos;

void main() {
	gl_Position = perspective * view * cameraTransformation * vec4(pos, 1);

	vec3 tangent = normalize(vec3(cameraTransformation * vec4(tangent, 0.0)));
	vec3 normal = normalize(vec3(cameraTransformation * vec4(0.0, 1.0, 0.0 , 0.0)));
	tangentBitangentNormal = transpose(mat3(tangent, cross(tangent, normal), normal));
	tangentLightPos = tangentBitangentNormal * lightPos;
	tangentViewPos = tangentBitangentNormal * cameraPos;
	tangentFragPos = tangentBitangentNormal * vec3(cameraTransformation * vec4(pos, 1));

	//texCoor = tex;
	texCoor = shadowMatrix * vec4(pos,1);
}