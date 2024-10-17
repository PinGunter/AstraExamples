#include "../../AstraEngine/AstraCore/shaders/wavefront.glsl"

struct hitPayload {
	vec3 hitValue;
	uint seed;
	uint depth;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	vec3 rayDir;
};