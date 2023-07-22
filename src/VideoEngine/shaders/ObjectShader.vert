#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat4 instanceTransform;

layout(push_constant) uniform MVP
{
	mat4 Model;
	mat4 InnerModel;
	mat4 ProjView;
} mvp;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPos;

void main() {
	mat4 toWorldTransform = mvp.Model * instanceTransform * mvp.InnerModel;

	gl_Position =
		mvp.ProjView * toWorldTransform * vec4(inPosition, 1.0);

	gl_Position.y *= -1;
	texCoord = inTexCoord;

	outNormal = (toWorldTransform * vec4(inNormal, 0.0f)).xyz;
	outPos = (toWorldTransform * vec4(inPosition, 1.0f)).xyz;
}
