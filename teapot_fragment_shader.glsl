#version 330 core

uniform sampler2DShadow tex;

out vec3 color;
in vec4 texCoor;
in vec4 transformedNormal;
in vec4 viewVector;
in vec4 light;

vec3 ambientColor = vec3(0.5, 0, 0);
float glossiness = 20; 
float intensity = 0.055;
float ambient_intensity = .75;

void main() {
	vec3 ambience = ambientColor * ambient_intensity;
	vec4 half_angle = normalize(light + viewVector);
	float specular = pow(max(dot(half_angle, transformedNormal), 0), glossiness);
	float nDotL = dot(transformedNormal, light);
	vec3 diffuseColor = vec3(0.1, 0.1, 0.1);
	vec3 specColor = vec3(0.5, 0.5, 0.5);
	vec3 blinn = intensity * nDotL * (diffuseColor + specColor * specular);
	color = vec3(blinn + ambience) * texture(tex, texCoor.xyz/texCoor.w) + ambientColor;

}