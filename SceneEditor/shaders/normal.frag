#version 450

layout(location = 0) in vec3 i_normal;

layout(location = 0) out vec4 fragColor;

void main()
{
	vec3 normal = normalize(i_normal);
	vec3 color = (normal+1.0f) / 2.0f;
	fragColor = vec4(color, 1.0f);
}