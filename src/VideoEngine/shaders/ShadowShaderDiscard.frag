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

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform Light
{
	int LightCount;
	layout(offset = 16) LightDescriptor Lights[100];
} light;

layout(set = 1, binding = 0) uniform sampler2D diffuseImage;

layout(push_constant) uniform LightPos {
	layout(offset = 192) int Index;
} lightPos;

void main() {
	vec4 objectColorAlpha = texture(diffuseImage, inTexCoord);

	if (objectColorAlpha.a == 0) {
		discard;
	}

	float farPlane = 500;
	vec3 lightPos = light.Lights[lightPos.Index].Position;

	float lightDistance = length(inPos.xyz - lightPos);
	lightDistance = lightDistance / farPlane;
	gl_FragDepth = lightDistance;
}
