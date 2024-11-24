
#pragma once

#include "Objects/Buffer.hpp"

#define ALIGN(size, alignment) (size + (alignment - 1)) & ~(alignment - 1)

namespace vulkan {
	class CommandBuffer;
	class AS {
	public:
		AS(void);
		~AS(void);

		void init_as(VkAccelerationStructureTypeKHR type);
		void build(vulkan::CommandBuffer& cb);

		vulkan::Buffer as_buffer;
		vulkan::Buffer scratch_buffer;

		std::vector< VkAccelerationStructureBuildRangeInfoKHR>	build_range_infos;
		VkAccelerationStructureBuildGeometryInfoKHR				build_geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		VkAccelerationStructureBuildSizesInfoKHR				build_sizes_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		VkAccelerationStructureKHR	ptr;
		VkDeviceAddress				address;
	};

	class BLAS : public AS {
	public:
		BLAS(void);
		~BLAS(void);

		void init(std::vector<vulkan::Buffer*> input_buffers);

		std::vector<VkAccelerationStructureGeometryKHR> geometries;
		std::vector<uint32_t> blas_max_primitive;


	};

	class TLAS : public AS {
	public:
		TLAS(void);
		~TLAS(void);
		
		void init(std::vector<BLAS> &blas);

		vulkan::Buffer instances_buffer;

		VkAccelerationStructureGeometryKHR geometry;
		std::vector<VkAccelerationStructureInstanceKHR> instances;
		 

	};

	class ShaderGroup {
	public:
		ShaderGroup(void) {
			group.anyHitShader = VK_SHADER_UNUSED_KHR;
			group.closestHitShader = VK_SHADER_UNUSED_KHR;
			group.generalShader = VK_SHADER_UNUSED_KHR;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;
		}

		~ShaderGroup(void) {}

		inline ShaderGroup& type_triangle(void) { group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR; return *this; };
		inline ShaderGroup& type_general(void) { group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR; return *this; };
		inline ShaderGroup& type_procedural(void) { group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR; return *this; };

		inline ShaderGroup& anyHitShader(uint32_t v) { group.anyHitShader = v; return *this; };
		inline ShaderGroup& closestHitShader(uint32_t v) { group.closestHitShader = v; return *this; };
		inline ShaderGroup& generalShader(uint32_t v) { group.generalShader = v; return *this; };
		inline ShaderGroup& intersectionShader(uint32_t v) { group.intersectionShader = v; return *this; };

		VkRayTracingShaderGroupCreateInfoKHR group{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
	};

	class ShaderBindingTable {
	public:
		ShaderBindingTable(void);
		~ShaderBindingTable(void);

		void init(VkPipeline pipeline, uint32_t miss_count, uint32_t hit_count);

		Buffer buffer;
		VkStridedDeviceAddressRegionKHR gen_region{};
		VkStridedDeviceAddressRegionKHR miss_region{};
		VkStridedDeviceAddressRegionKHR hit_region{};
		VkStridedDeviceAddressRegionKHR call_region{};
	};

	class RayTracingPipeline {
	public:
		RayTracingPipeline(void);
		~RayTracingPipeline(void);

		static void load_raytracing_functions(void);

		static PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
		static PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
		static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		static PFN_vkDestroyAccelerationStructureKHR pvkDestroyAccelerationStructureKHR;
		static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		static PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		static PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;

		inline void add_shader_stage(VkPipelineShaderStageCreateInfo stage) { stages.push_back(stage); }
		inline void add_shader_group(ShaderGroup group) { groups.push_back(group.group); }
		inline void add_layout(VkDescriptorSetLayout l) { layouts.push_back(l); }
		void init(void);


		std::vector<VkPipelineShaderStageCreateInfo> stages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
		std::vector<VkDescriptorSetLayout> layouts;

		VkPipelineLayout layout = nullptr;
		VkPipeline ptr = nullptr;
	private:
	};
}