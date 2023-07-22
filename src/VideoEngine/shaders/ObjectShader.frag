#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inPos;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Light
{
	vec3 Pos;
	vec3 Color;
} light;

void main() {
	outColor = texture(texSampler, texCoord);
}
