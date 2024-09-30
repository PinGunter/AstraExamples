#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "../../AstraEngine/AstraCore/shaders/wavefront.glsl"


layout(push_constant) uniform _PushConstantRaster
{
  PushConstantRaster pcRaster;
};

// clang-format off
// Incoming 
layout(location = 1) in vec3 i_worldPos;
layout(location = 2) in vec3 i_worldNrm;
layout(location = 3) in vec3 i_viewDir;
layout(location = 4) in vec2 i_texCoord;
// Outgoing
layout(location = 0) out vec4 o_color;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {uint i[]; }; // Triangle indices
layout(buffer_reference, scalar) buffer Materials {WaveFrontMaterial m[]; }; // Array of all materials on an object
layout(buffer_reference, scalar) buffer MatIndices {int i[]; }; // Material ID for each triangle

layout(binding = eObjDescs, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;
layout(binding = eTextures) uniform sampler2D[] textureSamplers;
layout(binding = eLights) uniform _LightsUniform { LightsUniform lightUni; };

// clang-format on


void main()
{
    float distanceL = 0.0f;
      vec3  L = vec3(0);
  // Material of the object
   for (int i=0; i < pcRaster.nLights; i++){
      vec3 N = normalize(i_worldNrm);

      // Vector toward light
      float lightIntensity = lightUni.lights[i].intensity;
      if(lightUni.lights[i].type == 0)
      {
        vec3  lDir     = lightUni.lights[i].position - i_worldPos;
        distanceL        = length(lDir);
        lightIntensity = lightUni.lights[i].intensity / (distanceL * distanceL);
        L              = normalize(lDir);

      } else {
        L = normalize(lightUni.lights[i].position);
      }

      o_color = vec4(vec3(max(dot(N, L), 0.2)), 1);

//      diffuseColor += computeDiffuse(mat, L, lightUni.lights[i].color, N) * lightIntensity;

    
  }

  // Result
}
