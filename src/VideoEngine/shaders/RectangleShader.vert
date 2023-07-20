#version 450

layout(push_constant) uniform MVP
{
	vec4 Pos;
	vec4 TexCoords;
} mvp;

layout(location = 0) out vec2 texCoord;

vec2 positions[6];
vec2 texCoords[6];

void main() {
	positions[0] = mvp.Pos.xy;
	positions[1] = mvp.Pos.xw;
	positions[2] = mvp.Pos.zy;
	positions[3] = mvp.Pos.zy;
	positions[4] = mvp.Pos.xw;
	positions[5] = mvp.Pos.zw;

	texCoords[0] = mvp.TexCoords.xy;
	texCoords[1] = mvp.TexCoords.xw;
	texCoords[2] = mvp.TexCoords.zy;
	texCoords[3] = mvp.TexCoords.zy;
	texCoords[4] = mvp.TexCoords.xw;
	texCoords[5] = mvp.TexCoords.zw;

	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	texCoord = texCoords[gl_VertexIndex];
}
