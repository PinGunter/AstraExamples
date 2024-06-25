#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

// this is a pain in the *** but since shaders should only be used by advanced users...
#include "../../AstraEngine/AstraCore/shaders/wavefront.glsl"


layout(location=0) in vec3 i_position;
layout(location=1) in vec3 i_normal;

layout(binding = eCamera) uniform _CameraUniform
{
  CameraUniform uni;
};

layout (location = 0) out vec3 o_normal;

layout(push_constant) uniform _PushConstantRaster
{
  PushConstantRaster pcRaster;
};

out gl_PerVertex
{
  vec4 gl_Position;
};


void main()
{
  vec3 worldPos = vec3(pcRaster.modelMatrix * vec4(i_position, 1.0));
  gl_Position = uni.viewProj * vec4(worldPos, 1.0);
  o_normal = i_normal;
}
