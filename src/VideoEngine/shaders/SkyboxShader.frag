#version 450

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube texSampler;

layout(push_constant) uniform MVP
{
	vec3 Direction;
	vec3 Up;
	vec3 ColorModifier;
	float FOV;
	float Ratio;
} mvp;

void main() {
	vec4 color = texture(texSampler, texCoord);
	outColor = vec4(color.rgb * mvp.ColorModifier, color.a);
}
