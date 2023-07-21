#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in mat4 instanceTransform;

layout(push_constant) uniform MVP
{
	mat4 Model;
	mat4 InnerModel;
	mat4 View;
	mat4 Proj;
} mvp;

layout(location = 0) out vec2 texCoord;

void main() {
	gl_Position =
		mvp.Proj * mvp.View *
		mvp.Model * instanceTransform * mvp.InnerModel *
		vec4(inPosition, 1.0);

	gl_Position.y *= -1;
	texCoord = inTexCoord;
}
