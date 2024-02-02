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

layout(set = 0, binding = 0) uniform Light
{
	int LightCount;
	layout(offset = 16) LightDescriptor Lights[100];
} light;

layout(push_constant) uniform LightPos {
	layout(offset = 192) int Index;
} lightPos;

void main() {
	float farPlane = 500;
	vec3 lightPos = light.Lights[lightPos.Index].Position;

	float lightDistance = length(inPos.xyz - lightPos);
	lightDistance = lightDistance / farPlane;
	gl_FragDepth = lightDistance;
}
