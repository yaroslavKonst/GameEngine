#version 450

struct LightDescriptor
{
	vec3 Position;
	vec3 Color;
	vec3 Direction;
	int Type;
	float Angle;
	float OuterAngle;
};

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 0) uniform Light
{
	int LightCount;
	layout(offset = 16) LightDescriptor Lights[100];
} light;

layout(set = 1, binding = 1) uniform samplerCube shadowSampler[100];

layout(push_constant) uniform ViewPos {
	layout(offset = 192) vec3 Pos;
	layout(offset = 204) int IsLight;
	layout(offset = 208) float LightMultiplier;
} viewPos;

float CalculateShadow(vec3 fragPos, vec3 viewPos, int lightIndex)
{
	float farPlane = 500;
	vec3 lightPos = light.Lights[lightIndex].Position;

	vec3 fragToLight = fragPos - lightPos;
	float currentDepth = length(fragToLight);

	vec3 sampleOffsetDirections[20] = vec3[]
	(
		vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1),
		vec3(-1,  1,  1),
		vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1),
		vec3(-1,  1, -1),
		vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0),
		vec3(-1,  1,  0),
		vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1),
		vec3(-1,  0, -1),
		vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1),
		vec3( 0,  1, -1)
	);

	float shadow = 0.0;
	float bias   = 0.05;
	int samples  = 20;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = 0.01;

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

vec3 ProcessPointLight(
	vec3 fragPos,
	vec3 lightPos,
	vec3 lightColor,
	vec3 viewPos,
	vec3 normal,
	int index)
{
	vec3 ambient = vec3(0.05f) * lightColor;

	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = lightColor * diff;

	float specularStrength = 0.5;
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float distance = length(lightPos - fragPos);
	float attenuation = 1.0f / (1.0f + 0.09f * distance +
		0.32f * (distance * distance));

	float shadow = CalculateShadow(fragPos, viewPos, index);
	return (ambient + diffuse + specular) * (1.0 - shadow) * attenuation;
}

vec3 ProcessSpotLight(
	vec3 fragPos,
	vec3 lightPos,
	vec3 lightColor,
	vec3 lightOrient,
	vec3 viewPos,
	vec3 normal,
	float lightAngle,
	float lightOuterAngle,
	int index)
{
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	vec3 ambient = vec3(0.05f) * lightColor;

	float distance = length(lightPos - fragPos);
	float attenuation = 1.0f / (1.0f + 0.09f * distance +
		0.32f * (distance * distance));

	float theta = dot(lightDir, normalize(-lightOrient));
	float epsilon = lightAngle - lightOuterAngle;
	float intensity = clamp((theta - lightOuterAngle) /
		epsilon, 0.0, 1.0);

	if (theta > lightOuterAngle) {
		float diff = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = lightColor * diff;

		float specularStrength = 0.5;
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
		vec3 specular = specularStrength * spec * lightColor;

		float shadow = CalculateShadow(fragPos, viewPos, index);
		return (ambient + (diffuse + specular) * intensity) *
			(1.0 - shadow) * attenuation;
	} else {
		return ambient * attenuation;
	}
}

void main() {
	vec4 objectColorAlpha = texture(texSampler, texCoord);
	vec3 objectColor = objectColorAlpha.rgb;

	if (viewPos.IsLight != 0) {
		outColor = vec4(
			objectColor * viewPos.LightMultiplier,
			objectColorAlpha.a);

		return;
	}

	vec3 normal = normalize(inNormal);

	vec3 sumLight = vec3(0.0f);

	for (int i = 0; i < light.LightCount; ++i) {
		if (light.Lights[i].Type == 0) {
			sumLight += ProcessPointLight(
				inPos,
				light.Lights[i].Position,
				light.Lights[i].Color,
				viewPos.Pos,
				normal,
				i);
		}

		if (light.Lights[i].Type == 1) {
			sumLight += ProcessSpotLight(
				inPos,
				light.Lights[i].Position,
				light.Lights[i].Color,
				light.Lights[i].Direction,
				viewPos.Pos,
				normal,
				light.Lights[i].Angle,
				light.Lights[i].OuterAngle,
				i);
		}
	}

	vec3 result = sumLight * objectColor;
	outColor = vec4(result, objectColorAlpha.a);
}
