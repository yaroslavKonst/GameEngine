#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler[2];

layout(push_constant) uniform HDR {
	float Exposure;
} hdr;

vec3 LightThr(vec3 color, float threshold)
{
	float sum = color.r + color.g + color.b;

	if (sum >= threshold) {
		return color;
	}

	return vec3(0.0f);
}

vec3 GetBlur(vec2 texCoords, float threshold)
{
	float weights[64] = float[] (
		1.0,   1.0,   0.25,  0.111, 0.062, 0.04,  0.027, 0.02,
		1.0,   0.499, 0.199, 0.099, 0.058, 0.038, 0.027, 0.019,
		0.25,  0.199, 0.124, 0.076, 0.049, 0.034, 0.024, 0.018,
		0.111, 0.099, 0.076, 0.055, 0.04,  0.029, 0.022, 0.017,
		0.062, 0.058, 0.049, 0.04,  0.031, 0.024, 0.019, 0.015,
		0.04,  0.038, 0.034, 0.029, 0.024, 0.019, 0.016, 0.013,
		0.027, 0.027, 0.024, 0.022, 0.019, 0.016, 0.013, 0.011,
		0.02,  0.019, 0.018, 0.017, 0.015, 0.013, 0.011, 0.01
	);

	float weight = 0.05;

	vec2 texOffset = 1.0 / textureSize(texSampler[0], 0);

	vec3 result = LightThr(
		texture(texSampler[0], texCoords).rgb,
		threshold) * weights[0] * weight;

	for(int i = 1; i < 8; ++i) {
		for (int j = 1; j < 8; ++j) {
			result += LightThr(
				texture(
					texSampler[0],
					texCoords +
					vec2(texOffset.x * i, 0.0) +
					vec2(0.0, texOffset.y * j)).rgb,
				threshold) * (weights[i * 8 + j] * weight);

			result += LightThr(
				texture(
					texSampler[0],
					texCoords +
					vec2(texOffset.x * i, 0.0) -
					vec2(0.0, texOffset.y * j)).rgb,
				threshold) * (weights[i * 8 + j] * weight);

			result += LightThr(
				texture(
					texSampler[0],
					texCoords -
					vec2(texOffset.x * i, 0.0) +
					vec2(0.0, texOffset.y * j)).rgb,
				threshold) * (weights[i * 8 + j] * weight);

			result += LightThr(
				texture(
					texSampler[0],
					texCoords -
					vec2(texOffset.x * i, 0.0) -
					vec2(0.0, texOffset.y * j)).rgb,
				threshold) * (weights[i * 8 + j] * weight);
		}
	}

	return result;
}

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(texSampler[0], texCoord).rgb;
	vec4 hdrInterface = texture(texSampler[1], texCoord);

	hdrColor += GetBlur(texCoord, 5.0f);

	vec3 mapped = vec3(1.0) - exp(-hdrColor * hdr.Exposure);

	// gamma correction
	//mapped = pow(mapped, vec3(1.0 / gamma));

	mapped = mapped * (1.0 - hdrInterface.a) +
		hdrInterface.rgb * hdrInterface.a;

	outColor = vec4(mapped, 1.0);
}
