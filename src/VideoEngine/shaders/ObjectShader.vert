#version 450

#include "ObjectInputArray.glsl"

layout(push_constant) uniform MVP
{
	mat4 Model;
	mat4 ProjView;
} mvp;

layout(set = 3, binding = 0) uniform InnerModelTransforms
{
	mat4 Transforms[20];
} InnerModel;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPos;

#include "DepthTransform.glsl"

void main() {
	mat4 toWorldTransform = mvp.Model * instanceTransform *
		InnerModel.Transforms[matrixIndex];

	gl_Position = mvp.ProjView * toWorldTransform * vec4(inPosition, 1.0);

	gl_Position.y *= -1;
	gl_Position.z = DepthTransform(gl_Position.z);
	texCoord = inTexCoord;

	outNormal = (toWorldTransform * vec4(inNormal, 0.0f)).xyz;
	outPos = (toWorldTransform * vec4(inPosition, 1.0f)).xyz;
}
