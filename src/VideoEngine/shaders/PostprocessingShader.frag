#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler[2];

layout(push_constant) uniform HDR {
	float Exposure;
} hdr;

void main() {
	const float gamma = 2.2;
	vec3 hdrColor = texture(texSampler[0], texCoord).rgb;
	vec4 hdrInterface = texture(texSampler[1], texCoord);

	vec3 mapped = vec3(1.0) - exp(-hdrColor * hdr.Exposure);

	// gamma correction
	//mapped = pow(mapped, vec3(1.0 / gamma));

	mapped = mapped * (1.0 - hdrInterface.a) +
		hdrInterface.rgb * hdrInterface.a;

	outColor = vec4(mapped, 1.0);
}
