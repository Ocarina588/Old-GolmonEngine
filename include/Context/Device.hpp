#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vulkan {

	class Device {
	public:
		friend class Context;

		Device(void);
		~Device(void);


		inline void add_extension(char const* str) { extensions.push_back(str); }
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

	private:

		void init(VkInstance instance, VkSurfaceKHR surface);

		void get_queues(VkSurfaceKHR surface);

		std::vector<char const*> extensions;
		uint32_t gpu_index = 0;
	};
}