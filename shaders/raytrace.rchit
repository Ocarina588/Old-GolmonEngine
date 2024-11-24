#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
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
layout(buffer_reference, scalar) buffer Vertices { Vertex data[]; };

layout(location = 0) rayPayloadInEXT hit_info_s hit_info; // Recursive ray payload

hitAttributeEXT vec3 attribs;

uint seed;

// Random number generator (improved for better distribution)
float rand() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return float(seed & 0x7FFFFFFF) / float(0x7FFFFFFF);
}

// Generate a random direction within a hemisphere around the given normal
vec3 randomHemisphereDirection(vec3 normal) {
    float u = rand();
    float v = rand();
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

void main() {
    // Fetch vertex data
    Vertices vertices = Vertices(unifo.adr[gl_InstanceCustomIndexEXT].vertex_address);
    int index = gl_PrimitiveID * 3;
    int material_index = int(unifo.adr[gl_InstanceCustomIndexEXT].material_index);

    Vertex v0 = vertices.data[index + 0];
    Vertex v1 = vertices.data[index + 1];
    Vertex v2 = vertices.data[index + 2];

    // Compute barycentric interpolation for position and normal
    vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 pos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
    vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));
    vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
    vec3 worldNormal = normalize(vec3(normal * gl_WorldToObjectEXT));
    
    // Offset to avoid self-intersections
    vec3 origin = worldPos + worldNormal * 0.1;

    // Generate a random diffuse direction
    vec3 direction = randomHemisphereDirection(worldNormal);

    // Decrement recursion step
    hit_info.recursive_step -= 1;

    // If recursion depth allows, cast another ray
    if (hit_info.recursive_step > 0)
        traceRayEXT(
            topLevelAS,
            gl_RayFlagsOpaqueEXT,
            0xFF,           // Mask
            0,              // SBT record offset
            1,              // SBT record stride
            0,              // Miss index
            origin,         // Ray origin
            1e-4,           // Min t
            direction,      // Ray direction
            1000.0,         // Max t
            0               // Payload location
        );
    else {
        // Accumulate lighting contribution
        hit_info.color = materials.tab[material_index].diffuse.xyz; // Modulate by diffuse
        hit_info.light += hit_info.color * materials.tab[material_index].emissive.xyz;
    }
  
}
