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

layout(set = 0, binding = 0) uniform sampler2D diffuseImage;
layout(set = 1, binding = 0) uniform sampler2D specularImage;

layout(set = 2, binding = 0) uniform Light
{
	int LightCount;
	layout(offset = 16) LightDescriptor Lights[100];
} light;

layout(set = 2, binding = 1) uniform samplerCube shadowSampler[100];

layout(push_constant) uniform ViewPos {
	layout(offset = 192) vec3 Pos;
	layout(offset = 204) int IsLight;
	layout(offset = 208) vec4 ColorMultiplier;
} viewPos;

#include "shadow.glsl"

#include "LightProcessing.glsl"

void main() {
	vec4 objectColorAlpha = texture(diffuseImage, texCoord);
	vec3 diffuseColor = objectColorAlpha.rgb;
	vec3 specularColor = texture(specularImage, texCoord).rgb;

	float viewDist = length(inPos - viewPos.Pos);

	if (viewPos.IsLight != 0) {
		outColor = vec4(diffuseColor, objectColorAlpha.a) *
			viewPos.ColorMultiplier;

		return;
	}

	vec3 normal = normalize(inNormal);

	vec3 sumLight = vec3(0.0f);

	for (int i = 0; i < light.LightCount; ++i) {
		if (light.Lights[i].Type == 0) {
			sumLight += ProcessPointLight(
				diffuseColor,
				specularColor,
				inPos,
				light.Lights[i].Position,
				light.Lights[i].Color,
				viewPos.Pos,
				normal,
				i);
		}

		if (light.Lights[i].Type == 1) {
			sumLight += ProcessSpotLight(
				diffuseColor,
				specularColor,
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

	vec3 result = sumLight;
	outColor = vec4(result, objectColorAlpha.a) * viewPos.ColorMultiplier;
}
