#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vulkan {

	class Device {
	public:
		friend class Context;

		Device(void);
		~Device(void);


		inline void add_extension(char const* str, void* vk_struct = nullptr) { extensions.push_back(str); if (vk_struct) vk_structs.push_back(vk_struct); }
		inline void set_gpu(uint32_t index) { gpu_index = index; }

		VkDevice ptr = nullptr;
		VkPhysicalDevice physical_ptr = nullptr;

		struct index_s {
			uint32_t graphics = ~0u;
			uint32_t compute = ~0u;
			uint32_t transfer = ~0u;
			uint32_t present = ~0u;
		} index;

		struct queue_s {
			VkQueue graphics = nullptr;
			VkQueue compute = nullptr;
			VkQueue transfer = nullptr;
			VkQueue present = nullptr;
		} queue;

		static VkMemoryRequirements get_memory_requirements(VkImage image);
		static VkMemoryRequirements get_memory_requirements(VkBuffer buffer);
		static uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkPhysicalDeviceBufferDeviceAddressFeatures device_address_feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR as_feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_pipeline_feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		VkPhysicalDeviceProperties2 properties_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };

	private:

		void init(VkInstance instance, VkSurfaceKHR surface);

		void get_queues(VkSurfaceKHR surface);

		std::vector<char const*> extensions;
		std::vector<void *> vk_structs;
		uint32_t gpu_index = 0;
	};
}