#version 330 core

uniform sampler2DShadow tex;

out vec4 color;
in vec4 texCoor;
in mat3 tangentBitangentNormal;

vec4 ambientColor = vec4(0.2, 0.2, 0.2, 1);

void main() {
	color = texture(tex, texCoor.xyz/texCoor.w) * ambientColor + ambientColor;
}