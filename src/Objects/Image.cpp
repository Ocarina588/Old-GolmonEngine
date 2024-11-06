#include "Golmon.hpp"

using namespace vulkan;

Image::Image(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties)
{
	init(extent, format, usage, aspect, properties);
}

Image::Image(void)
{

}

Image::~Image(void)
{
	vkDestroyImage(Context::device.ptr, image, nullptr);
	vkDestroyImageView(Context::device.ptr, view, nullptr);
	vkFreeMemory(Context::device.ptr, memory, nullptr);
}

void Image::init(VkExtent2D extent, VkFormat _format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties)
{
	format = _format;
	image = create(extent, format, usage);
	memory = create_memory(image, properties);
	if (vkBindImageMemory(Context::device.ptr, image, memory, 0) != VK_SUCCESS) throw std::runtime_error("failed to bind image");
	view = create_view(image, format, aspect);
}

VkImage Image::create(VkExtent2D extent, VkFormat format, VkImageUsageFlags usage)
{
	VkImageCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	VkImage image = nullptr;

	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent = { extent.width, extent.height, 1 };   
	create_info.mipLevels = 1;   
	create_info.arrayLayers = 1;
	create_info.format = format;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;

	if (vkCreateImage(Context::device.ptr, &create_info, nullptr, &image) != VK_SUCCESS) throw std::runtime_error("failed to create image");

	return image;
}

VkImageView Image::create_view(VkImage image, VkFormat format, VkImageAspectFlags aspect)
{
	VkImageViewCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	VkImageView view = nullptr;

	create_info.image = image;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = format;
	create_info.subresourceRange.aspectMask = aspect;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.layerCount = 1;

	if (vkCreateImageView(Context::device.ptr, &create_info, nullptr, &view) != VK_SUCCESS) throw std::runtime_error("failed to create image view");

	return view;
}

VkDeviceMemory Image::create_memory(VkImage image, VkMemoryPropertyFlags properties)
{
	auto memRequirements = Context::device.get_memory_requirements(image);

	VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Context::device.find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory imageMemory = nullptr;
	if (vkAllocateMemory(Context::device.ptr, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) throw std::runtime_error("failed to allocate memory");

	return imageMemory;
}

void Image::barrier(CommandBuffer& buffer, VkImageLayout old, VkImageLayout newl, VkPipelineStageFlags src, VkPipelineStageFlags dst)
{
	barrier(buffer, image, old, newl, src, dst);
}

void Image::barrier(CommandBuffer& buffer, VkImage image, VkImageLayout old, VkImageLayout newl, VkPipelineStageFlags src, VkPipelineStageFlags dst)
{
	VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	b.oldLayout = old;
	b.newLayout = newl;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = image;
	b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	b.subresourceRange.baseMipLevel = 0;
	b.subresourceRange.levelCount = 1;
	b.subresourceRange.baseArrayLayer = 0;
	b.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(buffer.ptr, src, dst, 0, 0, nullptr, 0, nullptr, 1, &b);
}


void Image::cpy(CommandBuffer& buffer, VkExtent2D extent, VkImage src, VkImage dst, VkImageLayout lsrc, VkImageLayout ldst)
{
	VkImageCopy copyRegion{};
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = copyRegion.srcSubresource;
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent.width = extent.width;
	copyRegion.extent.height = extent.height;
	copyRegion.extent.depth = 1;

	vkCmdCopyImage(buffer.ptr, src, lsrc, dst, ldst, 1, &copyRegion);
}