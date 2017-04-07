#version 330 core

uniform sampler2DShadow tex;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

out vec4 color;

in vec2 texCoor;
in mat3 tangentBitangentNormal;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

vec4 ambientColor = vec4(0.2, 0.2, 0.2, 1);

void main() {
	vec3 normal = texture(normalMap, texCoor).rgb;
	normal = normalize(normal * 2 - 1);
	vec3 texColor = texture(diffuseMap, texCoor).rgb;
	vec3 ambient = 0.1 * texColor;

	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * texColor;

	vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specVal = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = vec3(0.2) * specVal;

	color = vec4(ambient + diffuse + specular, 1.0f);

	//color = texture(tex, texCoor.xyz/texCoor.w) * ambientColor + ambientColor;
}