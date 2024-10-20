#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout (push_constant) uniform _PushConstantRay {
    PushConstantRay pcRay;
};

void main()
{
    if (prd.depth == 0)
    {
        const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
        const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
        prd.hitValue = mix( pcRay.clearColor.xyz, pcRay.clearColor2.xyz, inUV.y);    
    } 
    else 
    {
        prd.hitValue = 0.3 * pcRay.clearColor.xyz;
    }
    prd.depth = 100;    
}