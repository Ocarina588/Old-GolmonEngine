#ifndef COMMON_HOST_DEVICE
#define COMMON_HOST_DEVICE

#ifdef __cplusplus

#include <glm/glm.hpp>

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

using uint = unsigned int;

#endif

#ifdef __cplusplus
 #define START_BINDING(a) enum a {
 #define END_BINDING() }
#else
 #define START_BINDING(a)  const uint
 #define END_BINDING() 
#endif

START_BINDING(RtxBindings)
  eTlas			= 0, 
  eOutImage		= 1, 
  eCamera		= 2,
  eVertexBuffer = 3
END_BINDING();


struct instance_info_s
{
	uint64_t vertex_address;
	uint64_t material_index;
};

struct material_info_s
{
	vec4 diffuse;
	vec4 specular;
	vec4 emissive;
};

struct camera_info_s
{
	mat4 viewProj;     // Camera view * projection
	mat4 viewInverse;  // Camera inverse view matrix
	mat4 projInverse;  // Camera inverse projection matrix
};

struct hit_info_s
{
	vec3 color;
	vec3 light;
	int recursive_step;
	vec3 pos;
	vec3 direction;
};

#endif
