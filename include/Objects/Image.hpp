#pragma once
#include "Objects/Commands.hpp"
#include "stb_image.h"

namespace vulkan {

	class Sampler {
	public:
		Sampler(void);
		~Sampler(void) {};

		void init(void);
	
		VkSamplerCreateInfo create_info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		VkSampler ptr = nullptr;
	};

	class Image {
	public:

		Image(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		Image(void);
		~Image(void);

		void init(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		void init_compressed(vulkan::CommandBuffer& co, void* data, uint32_t size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		static VkImage create(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage);
		static VkImageView create_view(VkImage image, VkFormat format, VkImageAspectFlags aspect);
		static VkDeviceMemory create_memory(VkImage image, VkMemoryPropertyFlags properties);

		void barrier(CommandBuffer& buffer, VkImageLayout old, VkImageLayout newl, VkPipelineStageFlags src, VkPipelineStageFlags dst);
		static void barrier(CommandBuffer& buffer, VkImage image, VkImageLayout old, VkImageLayout newl, VkPipelineStageFlags src, VkPipelineStageFlags dst);
		static void cpy(CommandBuffer& buffer, VkExtent2D extent, VkImage src, VkImage dst, VkImageLayout lsrc, VkImageLayout ldst);
		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkDeviceMemory memory = nullptr;
		VkFormat format;
	private:
	};
}