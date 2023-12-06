#version 450

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform samplerCube texSampler1;
layout(set = 1, binding = 0) uniform samplerCube texSampler2;

layout(push_constant) uniform MVP
{
	vec3 Direction;
	vec3 Up;
	vec3 ColorModifier1;
	vec3 ColorModifier2;
	vec3 Gradient;
	float FOV;
	float Ratio;
	int GradientEnabled;
	float GradientOffset;
} mvp;

void main() {
	if (mvp.GradientEnabled != 0) {
		vec4 color1 = texture(texSampler1, texCoord);
		vec4 color2 = texture(texSampler2, texCoord);

		float coeff = dot(mvp.Gradient, normalize(texCoord));
		coeff = (clamp(coeff + mvp.GradientOffset, -1.0, 1.0) + 1.0) /
			2.0;

		color1 = vec4(color1.rgb * mvp.ColorModifier1, color1.a);
		color2 = vec4(color2.rgb * mvp.ColorModifier2, color2.a);

		outColor = color1 * coeff + color2 * (1.0 - coeff);
	} else {
		vec4 color = texture(texSampler1, texCoord);
		outColor = vec4(color.rgb * mvp.ColorModifier1, color.a);
	}
}
