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

void main()
{
	mat4 innerMatrix;
	innerMatrix[0] = vec4(0.0);
	innerMatrix[1] = vec4(0.0);
	innerMatrix[2] = vec4(0.0);
	innerMatrix[3] = vec4(0.0);

	for (int i = 0; i < 2; ++i) {
		innerMatrix += InnerModel.Transforms[matrixIndex[i]] *
			matrixCoeff[i];
	}

	mat4 toWorldTransform = mvp.Model * instanceTransform * innerMatrix;

	gl_Position = toWorldTransform * vec4(inPosition, 1.0);
	outTexCoord = inTexCoord;
}
