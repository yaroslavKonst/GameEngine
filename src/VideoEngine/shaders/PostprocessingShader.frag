#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform HDR {
	float Exposure;
} hdr;

void main() {
	const float gamma = 2.2;
	vec3 hdrColor = texture(texSampler, texCoord).rgb;

	vec3 mapped = vec3(1.0) - exp(-hdrColor * hdr.Exposure);

	// gamma correction
	//mapped = pow(mapped, vec3(1.0 / gamma));

	outColor = vec4(mapped, 1.0);
}
