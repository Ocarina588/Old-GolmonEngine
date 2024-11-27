#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
#extension GL_EXT_buffer_reference2 : require

#include "host_device.h"

struct Vertex {
    vec3 pos;
    float padding1;
    vec3 normal;
    float padding2;
};

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 3) buffer _ { instance_info_s adr[]; } unifo;
layout(set = 0, binding = 4, scalar) buffer _d { material_info_s tab[]; } materials;
layout(set = 0, binding = eCamera) uniform _ { camera_info_s camera; };

layout(buffer_reference, scalar) buffer Vertices { Vertex data[]; };

layout(location = 0) rayPayloadInEXT hitPayload prd; // Recursive ray payload

hitAttributeEXT vec3 attribs;

#define M_PI 3.1415926535897932384626433832795

uint PCG_Hash(uint ino)
{
    uint state = ino * 747796405 + 2891336453;
    uint word = (((state >> (state >> 28) + 4)) ^ state) * 277803737;
    return (word >> 22) ^ word;
}

float rand(float seed) {
  return fract(sin(mod(dot(gl_LaunchIDEXT.xy, vec2(12.9898, 78.233)) + 1113.1 * seed, M_PI)) *
               43758.5453);
  ;
}

float random(float trash)
{
    prd.seed = PCG_Hash(prd.seed);
    return prd.seed / float(0x7FFFFFFF);
}

// Generate a random direction within a hemisphere around the given normal
vec3 randomHemisphereDirection(vec3 normal) {
    float u = random(camera.frames.x);
    float v = random(camera.frames.x + 1);
    float theta = 2.0 * 3.14159265359 * u; // Azimuthal angle
    float phi = acos(2.0 * v - 1.0);       // Elevation angle

    vec3 localDir = vec3(
        cos(theta) * sin(phi),
        sin(theta) * sin(phi),
        cos(phi)
    );

    // Transform to world space using the normal
    vec3 tangent = normalize(cross(abs(normal.z) > 0.1 ? vec3(1, 0, 0) : vec3(0, 0, 1), normal));
    vec3 bitangent = cross(normal, tangent);
    return normalize(localDir.x * tangent + localDir.y * bitangent + localDir.z * normal);
}

vec3 random_dir(vec3 normal)
{
    float phi = 2.f * 3.14159265358979323846264338327950288 * random(camera.frames.x);
    float cosTheta = random(camera.frames.x + 1);
    float sinTheta = sqrt(1.f - cosTheta * cosTheta);

    vec3 local_direction = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    vec3 tangent = abs(normal.x) > 0.99f ? vec3(0.f, 1.f, 0.f) : vec3(1.f, 0.f, 0.f);
    tangent = normalize(cross(normal, tangent));
    vec3 bitangent = cross(normal, tangent);
    return normalize(local_direction.x * tangent + local_direction.y * bitangent + local_direction.z * normal);
}

void main() {
    // Fetch vertex data
    //seed = 10;

    int material_index = int(unifo.adr[gl_InstanceCustomIndexEXT].material_index);
    int vertices_index = gl_PrimitiveID * 3;
    material_info_s material = materials.tab[material_index];
    Vertices        vertices = Vertices(unifo.adr[gl_InstanceCustomIndexEXT].vertex_address);
    Vertex v0 = vertices.data[vertices_index + 0];
    Vertex v1 = vertices.data[vertices_index + 1];
    Vertex v2 = vertices.data[vertices_index + 2];

    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    const vec3 pos      = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
    const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space
    const vec3 nrm      = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
    const vec3 worldNrm = normalize(vec3(nrm * gl_WorldToObjectEXT));  // Transforming the normal to world space

    vec3 specular_direction = reflect(gl_WorldRayDirectionEXT, worldNrm);
    vec3 diffuse_direction = random_dir(worldNrm);
    bool is_specular_direction = material.smoooth.y >= random(prd.seed);

    prd.rayOrigin = worldPos;
    prd.rayDir    = mix(diffuse_direction, specular_direction, is_specular_direction ? material.smoooth.x : 0);
    prd.material  = materials.tab[material_index];
    prd.color = worldNrm;
}
