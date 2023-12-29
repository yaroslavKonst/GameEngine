#version 450

#include "ObjectInputArray.glsl"

layout(push_constant) uniform MVP
{
	mat4 Model;
	mat4 ProjView;
} mvp;

layout(set = 1, binding = 0) uniform InnerModelTransforms
{
	mat4 Transforms[20];
} InnerModel;

void main() {
	mat4 toWorldTransform = mvp.Model * instanceTransform *
		InnerModel.Transforms[matrixIndex];

	gl_Position = toWorldTransform * vec4(inPosition, 1.0);
}
