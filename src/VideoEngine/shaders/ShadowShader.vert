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

void main() {
	mat4 toWorldTransform = mvp.Model * instanceTransform * mvp.InnerModel;

	gl_Position = toWorldTransform * vec4(inPosition, 1.0);
}
