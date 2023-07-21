#version 450

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube texSampler;

void main() {
	outColor = texture(texSampler, texCoord);
}
