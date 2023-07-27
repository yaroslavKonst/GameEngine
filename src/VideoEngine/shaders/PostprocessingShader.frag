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
	float weight[5] = float[] (
		0.227027,
		0.1945946,
		0.1216216,
		0.054054,
		0.016216);

	vec2 texOffset = 1.0 / textureSize(texSampler[0], 0);

	vec3 result = LightThr(
		texture(texSampler[0], texCoords).rgb * weight[0],
		threshold);

	for(int i = 1; i < 5; ++i) {
		result += LightThr(
			texture(texSampler[0], texCoords +
				vec2(texOffset.x * i, 0.0)).rgb * weight[i],
			threshold);
		result += LightThr(
			texture(texSampler[0], texCoords -
				vec2(texOffset.x * i, 0.0)).rgb * weight[i],
			threshold);
	}

	for(int i = 1; i < 5; ++i) {
		result += LightThr(
			texture(texSampler[0], texCoords +
				vec2(0.0, texOffset.y * i)).rgb * weight[i],
			threshold);
		result += LightThr(
			texture(texSampler[0], texCoords -
				vec2(0.0, texOffset.y * i)).rgb * weight[i],
			threshold);
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
