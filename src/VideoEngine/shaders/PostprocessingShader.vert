#version 450

layout(location = 0) out vec2 texCoord;

vec2 positions[6];
vec2 texCoords[6];

void main() {
	positions[0] = vec2(-1.0f, -1.0f);
	positions[1] = vec2(-1.0f, 1.0f);
	positions[2] = vec2(1.0f, 1.0f);

	positions[3] = vec2(-1.0f, -1.0f);
	positions[4] = vec2(1.0f, 1.0f);
	positions[5] = vec2(1.0f, -1.0f);

	texCoords[0] = vec2(0.0f, 0.0f);
	texCoords[1] = vec2(0.0f, 1.0f);
	texCoords[2] = vec2(1.0f, 1.0f);

	texCoords[3] = vec2(0.0f, 0.0f);
	texCoords[4] = vec2(1.0f, 1.0f);
	texCoords[5] = vec2(1.0f, 0.0f);

	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	texCoord = texCoords[gl_VertexIndex];
}
