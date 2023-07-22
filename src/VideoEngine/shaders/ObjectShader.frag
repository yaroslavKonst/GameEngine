#version 450

struct LightDescriptor
{
	vec3 Position;
	vec3 Color;
	int Type;
};

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;


layout(set = 1, binding = 1) uniform Light
{
	int LightCount;
	layout(offset = 16) LightDescriptor Lights[100];
} light;

layout(push_constant) uniform ViewPos {
	layout(offset = 192) vec3 Pos;
} viewPos;

vec3 ProcessPointLight(
	vec3 fragPos,
	vec3 lightPos,
	vec3 lightColor,
	vec3 viewPos,
	vec3 normal)
{
	vec3 ambient = vec3(0.05f) * lightColor;

	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = lightColor * diff;

	float specularStrength = 0.5;
	float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float distance = length(lightPos - fragPos);
	float attenuation = 1.0f / (1.0f + 0.09f * distance +
		0.32f * (distance * distance));

	return (ambient + diffuse + specular) * attenuation;
}

void main() {
	vec3 objectColor = (texture(texSampler, texCoord)).rgb;
	vec3 normal = normalize(inNormal);

	vec3 sumLight = vec3(0.0f);

	for (int i = 0; i < light.LightCount; ++i) {
		sumLight += ProcessPointLight(
			inPos,
			light.Lights[i].Position,
			light.Lights[i].Color,
			viewPos.Pos,
			normal);
	}

	vec3 result = sumLight * objectColor;
	outColor = vec4(result, 1.0f);
}
