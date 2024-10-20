#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_debug_printf : enable

#include "common.glsl"
#include "sampling.glsl"

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {ivec3 i[]; }; // Triangle indices
layout(buffer_reference, scalar) buffer Materials {WaveFrontMaterial m[]; }; // Array of all materials on an object
layout(buffer_reference, scalar) buffer MatIndices {int i[]; }; // Material ID for each triangle
layout(set = 0, binding = eTlas) uniform accelerationStructureEXT topLevelAS;
layout(set = 1, binding = eObjDescs, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;
layout(set = 1, binding = eTextures) uniform sampler2D textureSamplers[];
layout(set = 1, binding = eLights) uniform _LightsUniform { LightsUniform lightUni; };

layout(push_constant) uniform _PushConstantRay { PushConstantRay pcRay; };

void main()
{
	// object data
	ObjDesc objResource = objDesc.i[gl_InstanceCustomIndexEXT];
	MatIndices matIndices = MatIndices(objResource.materialIndexAddress);
	Materials materials = Materials (objResource.materialAddress);
	Indices indices = Indices(objResource.indexAddress);
	Vertices vertices = Vertices(objResource.vertexAddress);

	// indices of the triangle
	ivec3 ind = indices.i[gl_PrimitiveID];

	// vertex of the triangle
	Vertex v0 = vertices.v[ind.x];
	Vertex v1 = vertices.v[ind.y];
	Vertex v2 = vertices.v[ind.z];
	
	const vec3 barycentrics = vec3(1.0 - attribs.x  -attribs.y, attribs.x, attribs.y);
	
	// Computing the coordinates of the hit position
	const vec3 pos      = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
	const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

	// Computing normal
	const vec3 nrm = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
	vec3 worldNrm = normalize(vec3(nrm * gl_WorldToObjectEXT));

	// Material of the object
	int matIdx = matIndices.i[gl_PrimitiveID];
	WaveFrontMaterial mat = materials.m[matIdx];
    vec3 emittance = mat.emission;

	vec3 tangent, bitangent;
	createCoordinateSystem(worldNrm, tangent, bitangent);
	vec3 rayOrigin = worldPos;
	vec3 rayDirection = samplingHemisphere(prd.seed, tangent, bitangent, worldNrm);

	const float cos_theta = dot(rayDirection, worldNrm);
	const float p = cos_theta / M_PI;

	vec3 albedo = mat.diffuse;

	
	 if (mat.shininess >= 1000.0f){
		rayDirection = reflect(gl_WorldRayDirectionEXT, worldNrm);
	 }

	if (mat.textureId > -1){
		 uint txtId    = mat.textureId + objDesc.i[gl_InstanceCustomIndexEXT].txtOffset;
         vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;
         albedo *= texture(textureSamplers[nonuniformEXT(txtId)], texCoord).xyz;
	}

	 vec3 BRDF = albedo / M_PI;



	if (prd.depth < 10)
	  {
		prd.depth++;
		float tMin = 0.001;
		float tMax = 100000000.0;
		uint flags = gl_RayFlagsOpaqueEXT;
		traceRayEXT(topLevelAS,flags,0xFF,0,0,0,rayOrigin,tMin,rayDirection, tMax, 0);
	  }
	
	vec3 incoming = prd.hitValue;

	prd.hitValue = emittance + (BRDF * incoming * cos_theta/p);
}

