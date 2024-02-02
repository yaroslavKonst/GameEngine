#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(set = 0, binding = 0) uniform Shadow
{
	layout(offset = 1024) mat4 Matrices[60];
} shadow;

layout(push_constant) uniform LightPos {
	layout(offset = 192) int Index;
} lightPos;

layout(location = 0) out vec4 FragPos;

void main()
{
	for(int face = 0; face < 6; ++face) {
		gl_Layer = face;

		for(int i = 0; i < 3; ++i) {
			FragPos = gl_in[i].gl_Position;
			gl_Position = shadow.Matrices[
				face + 6 * lightPos.Index] * FragPos;
			EmitVertex();
		}

		EndPrimitive();
	}
}
