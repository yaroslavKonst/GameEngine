#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(push_constant) uniform MVP
{
	mat4 Model;
	mat4 View;
	mat4 Proj;
} mvp;

layout(location = 0) out vec2 texCoord;

void main() {
	gl_Position =
		mvp.Proj * mvp.View * mvp.Model * vec4(inPosition, 0.0, 1.0);
	texCoord = inTexCoord;
}
