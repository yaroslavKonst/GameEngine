#version 450

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube texSampler;

layout(push_constant) uniform MVP
{
	vec3 Direction;
	vec3 Up;
	vec3 ColorModifier;
	vec3 Gradient;
	float FOV;
	float Ratio;
	int GradientEnabled;
	float GradientOffset;
} mvp;

void main() {
	if (mvp.GradientEnabled != 0) {
		vec4 color = texture(texSampler, texCoord);

		float coeff = dot(mvp.Gradient, normalize(texCoord));
		coeff = (clamp(coeff + mvp.GradientOffset, -1.0, 1.0) + 1.0) /
			2.0;

		color = vec4(color.rgb * mvp.ColorModifier, 0.0);
		outColor = color * coeff;
	} else {
		vec4 color = texture(texSampler, texCoord);
		outColor = vec4(color.rgb * mvp.ColorModifier, 0.0);
	}
}
