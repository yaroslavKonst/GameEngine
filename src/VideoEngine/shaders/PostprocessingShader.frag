#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler[2];

layout(binding = 1) buffer ExposureBuffer {
	float Values[2];
} exposureBuffer;

layout(push_constant) uniform HDR {
	float MinExposure;
	float MaxExposure;
	int Index;
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

void CorrectExposure(float exposure)
{
	vec2 texSize = textureSize(texSampler[0], 0);

	if (ivec2(gl_FragCoord.xy) == ivec2(texSize / 2)) {
		vec3 avgLight = vec3(0.0f);

		float coords[9] = float[] (
			0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9
		);

		for (int i = 0; i < 9; ++i) {
			for (int j = 0; j < 9; ++j) {
				vec3 color = texture(
					texSampler[0],
					vec2(coords[i], coords[j])).rgb;

				avgLight += vec3(1.0) -
					exp(-color * exposure);
			}
		}

		avgLight /= 81.0f;

		float value = max(max(avgLight.r, avgLight.g), avgLight.b);

		if (value > 0.8) {
			exposure -= 0.005;
		}

		if (value < 0.8) {
			exposure += 0.005;
		}

		exposure = clamp(exposure, hdr.MinExposure, hdr.MaxExposure);

		exposureBuffer.Values[(hdr.Index + 1) % 2] = exposure;
	}
}

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(texSampler[0], texCoord).rgb;
	vec4 hdrInterface = texture(texSampler[1], texCoord);

	hdrColor += GetBlur(texCoord, 5.0f);

	float exposure = clamp(
		exposureBuffer.Values[hdr.Index],
		hdr.MinExposure,
		hdr.MaxExposure);

	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	CorrectExposure(exposure);

	// gamma correction
	//mapped = pow(mapped, vec3(1.0 / gamma));

	mapped = mapped * (1.0 - hdrInterface.a) +
		hdrInterface.rgb * hdrInterface.a;

	outColor = vec4(mapped, 1.0);

	if (int(gl_FragCoord.y) == int(exposure * 40.0f) &&
		gl_FragCoord.x < 20)
	{
		outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}

	if (int(gl_FragCoord.y) == int(exposure * 40.0f) + 1 &&
		gl_FragCoord.x < 20)
	{
		outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	if (int(gl_FragCoord.y) == int(exposure * 40.0f) + 2 &&
		gl_FragCoord.x < 20)
	{
		outColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
}
