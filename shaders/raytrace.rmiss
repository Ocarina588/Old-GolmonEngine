#version 460
#extension GL_EXT_ray_tracing : require

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "host_device.h"

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{
    prd.material.emissive = vec4(0);
    prd.material.diffuse = vec4(0);
}