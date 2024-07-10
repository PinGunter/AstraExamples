#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "raypayload.glsl"
#include "../../AstraEngine/AstraCore/shaders/host_device.h"

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout (push_constant) uniform _PushConstantRay {
    PushConstantRay pcRay;
};

void main()
{
  if (pcRay.maxDepth == 0){
    prd.hitValue = pcRay.clearColor.xyz * 0.8;
  } else{
    prd.hitValue = vec3(0.01); // if it misses, no contribution
  }
  prd.depth = 100; // stop tracing
}