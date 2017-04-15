#version 330 core

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

out vec4 color;

in vec2 texCoor;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

float height_scale = 0.4;

vec4 ambientColor = vec4(0.2, 0.2, 0.2, 1);

vec2 getParallaxCoors(vec2 texCoor, vec3 viewDir){
	float height = texture(depthMap, texCoor).r;
	vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
	return texCoor - p;
}

void main() {
	vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
	vec2 parallaxCoor = getParallaxCoors(texCoor, viewDir);

	vec3 normal = texture(normalMap, parallaxCoor).rgb;
	normal = normalize(normal * 2 - 1);

	vec3 texColor = texture(diffuseMap, parallaxCoor).rgb;
	vec3 ambient = 0.1 * texColor;

	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * texColor;


	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specVal = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = vec3(0.2) * specVal;

	color = vec4(ambient + diffuse + specular, 1.0f);

}