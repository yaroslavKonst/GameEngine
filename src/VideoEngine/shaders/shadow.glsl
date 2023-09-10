float CalculateShadow(vec3 fragPos, vec3 viewPos, int lightIndex)
{
	float farPlane = 500;
	vec3 lightPos = light.Lights[lightIndex].Position;

	vec3 fragToLight = fragPos - lightPos;
	float currentDepth = length(fragToLight);

	vec3 sampleOffsetDirections[52] = vec3[]
	(
		vec3(-1, -1, -1),
		vec3(-1, -1, 0),
		vec3(-1, -1, 1),
		vec3(-1, 0, -1),
		vec3(-1, 0, 0),
		vec3(-1, 0, 1),
		vec3(-1, 1, -1),
		vec3(-1, 1, 0),
		vec3(-1, 1, 1),

		vec3(0, -1, -1),
		vec3(0, -1, 0),
		vec3(0, -1, 1),
		vec3(0, 0, -1),
		vec3(0, 0, 1),
		vec3(0, 1, -1),
		vec3(0, 1, 0),
		vec3(0, 1, 1),

		vec3(1, -1, -1),
		vec3(1, -1, 0),
		vec3(1, -1, 1),
		vec3(1, 0, -1),
		vec3(1, 0, 0),
		vec3(1, 0, 1),
		vec3(1, 1, -1),
		vec3(1, 1, 0),
		vec3(1, 1, 1),

		vec3(-1, -1, -1) * 0.5,
		vec3(-1, -1, 0) * 0.5,
		vec3(-1, -1, 1) * 0.5,
		vec3(-1, 0, -1) * 0.5,
		vec3(-1, 0, 0) * 0.5,
		vec3(-1, 0, 1) * 0.5,
		vec3(-1, 1, -1) * 0.5,
		vec3(-1, 1, 0) * 0.5,
		vec3(-1, 1, 1) * 0.5,

		vec3(0, -1, -1) * 0.5,
		vec3(0, -1, 0) * 0.5,
		vec3(0, -1, 1) * 0.5,
		vec3(0, 0, -1) * 0.5,
		vec3(0, 0, 1) * 0.5,
		vec3(0, 1, -1) * 0.5,
		vec3(0, 1, 0) * 0.5,
		vec3(0, 1, 1) * 0.5,

		vec3(1, -1, -1) * 0.5,
		vec3(1, -1, 0) * 0.5,
		vec3(1, -1, 1) * 0.5,
		vec3(1, 0, -1) * 0.5,
		vec3(1, 0, 0) * 0.5,
		vec3(1, 0, 1) * 0.5,
		vec3(1, 1, -1) * 0.5,
		vec3(1, 1, 0) * 0.5,
		vec3(1, 1, 1) * 0.5
	);

	float shadow = 0.0;
	float bias   = 0.05;
	int samples  = 52;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = 0.01 + (currentDepth / farPlane);

	for (int i = 0; i < samples; ++i) {
		float closestDepth = texture(
			shadowSampler[lightIndex],
			fragToLight + sampleOffsetDirections[i] * diskRadius).r;

		if (closestDepth == 1.0) {
			continue;
		}

		closestDepth *= farPlane;

		if (currentDepth - bias > closestDepth) {
			shadow += 1.0;
		}
	}

	shadow /= float(samples);

	return shadow;
}
