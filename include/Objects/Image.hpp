#pragma once
#include "Golmon.hpp"

namespace vulkan {

	class Image {
	public:
		Image(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		Image(void);
		~Image(void);

		void init(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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