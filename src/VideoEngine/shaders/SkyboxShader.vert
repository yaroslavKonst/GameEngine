#version 450

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

layout(location = 0) out vec3 texCoord;

vec2 positions[6];
vec3 texCoords[6];

void main() {
	vec3 horOffset = normalize(cross(mvp.Direction, mvp.Up));
	vec3 up = normalize(cross(horOffset, mvp.Direction));

	vec3 vertOffset = up * tan(mvp.FOV / 2.0) * length(mvp.Direction);

	horOffset *= length(vertOffset) * mvp.Ratio;

	positions[0] = vec2(-1.0f, -1.0f);
	positions[1] = vec2(-1.0f, 1.0f);
	positions[2] = vec2(1.0f, 1.0f);

	positions[3] = vec2(-1.0f, -1.0f);
	positions[4] = vec2(1.0f, 1.0f);
	positions[5] = vec2(1.0f, -1.0f);

	texCoords[0] = normalize(mvp.Direction + vertOffset - horOffset);
	texCoords[1] = normalize(mvp.Direction - vertOffset - horOffset);
	texCoords[2] = normalize(mvp.Direction - vertOffset + horOffset);

	texCoords[3] = normalize(mvp.Direction + vertOffset - horOffset);
	texCoords[4] = normalize(mvp.Direction - vertOffset + horOffset);
	texCoords[5] = normalize(mvp.Direction + vertOffset + horOffset);

	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	texCoord = texCoords[gl_VertexIndex];
}
