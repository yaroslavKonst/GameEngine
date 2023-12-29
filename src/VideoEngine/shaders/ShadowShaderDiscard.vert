#version 450

#include "ObjectInputArray.glsl"

layout(location = 0) out vec2 outTexCoord;

layout(push_constant) uniform MVP
{
	mat4 Model;
	mat4 ProjView;
} mvp;

layout(set = 2, binding = 0) uniform InnerModelTransforms
{
	mat4 Transforms[20];
} InnerModel;

void main() {
	mat4 toWorldTransform = mvp.Model * instanceTransform *
		InnerModel.Transforms[matrixIndex];

	gl_Position = toWorldTransform * vec4(inPosition, 1.0);
	outTexCoord = inTexCoord;
}
