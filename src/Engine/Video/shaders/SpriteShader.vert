#version 450

layout(push_constant) uniform MVP
{
	mat4 ProjView;
	vec4 TexCoords;
	vec3 SpritePos;
	vec3 CameraPos;
	vec3 SpriteUp;
	vec2 Size;
} mvp;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPos;

vec4 positions[6];
vec3 positionsWorld[6];
vec2 texCoords[6];

#include "DepthTransform.glsl"

void main() {
	vec3 pos[4];
	vec4 posProj[4];

	vec3 viewDirection = normalize(mvp.SpritePos - mvp.CameraPos);
	vec3 horOffset = normalize(cross(viewDirection, mvp.SpriteUp));
	vec3 vertOffset = normalize(cross(horOffset, viewDirection));

	horOffset *= mvp.Size.x / 2.0f;
	vertOffset *= mvp.Size.y / 2.0f;

	pos[0] = mvp.SpritePos - horOffset + vertOffset;
	pos[1] = mvp.SpritePos - horOffset - vertOffset;
	pos[2] = mvp.SpritePos + horOffset + vertOffset;
	pos[3] = mvp.SpritePos + horOffset - vertOffset;

	posProj[0] = mvp.ProjView * vec4(pos[0], 1.0);
	posProj[1] = mvp.ProjView * vec4(pos[1], 1.0);
	posProj[2] = mvp.ProjView * vec4(pos[2], 1.0);
	posProj[3] = mvp.ProjView * vec4(pos[3], 1.0);

	positionsWorld[0] = pos[0]; // 0
	positionsWorld[1] = pos[1]; // 1
	positionsWorld[2] = pos[2]; // 2
	positionsWorld[3] = pos[2]; // 2
	positionsWorld[4] = pos[1]; // 1
	positionsWorld[5] = pos[3]; // 3

	positions[0] = posProj[0]; // 0
	positions[1] = posProj[1]; // 1
	positions[2] = posProj[2]; // 2
	positions[3] = posProj[2]; // 2
	positions[4] = posProj[1]; // 1
	positions[5] = posProj[3]; // 3

	texCoords[0] = mvp.TexCoords.xy;
	texCoords[1] = mvp.TexCoords.xw;
	texCoords[2] = mvp.TexCoords.zy;
	texCoords[3] = mvp.TexCoords.zy;
	texCoords[4] = mvp.TexCoords.xw;
	texCoords[5] = mvp.TexCoords.zw;

	gl_Position = positions[gl_VertexIndex];

	gl_Position.y *= -1;
	gl_Position.z = DepthTransform(gl_Position.z);

	texCoord = texCoords[gl_VertexIndex];
	outPos = positionsWorld[gl_VertexIndex];
	outNormal = normalize(cross(horOffset, vertOffset));
}
