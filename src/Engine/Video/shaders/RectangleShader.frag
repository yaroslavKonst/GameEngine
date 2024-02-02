#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Color {
	layout(offset = 32) vec4 Multiplier;
} color;

void main() {
	outColor = texture(texSampler, texCoord) * color.Multiplier;
}
