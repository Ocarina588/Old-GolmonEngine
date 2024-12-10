#define STB_IMAGE_IMPLEMENTATION
#include "Golmon.hpp"

using namespace vulkan;

Sampler::Sampler(void)
{
	create_info.magFilter = VK_FILTER_LINEAR;
	create_info.minFilter = VK_FILTER_LINEAR;
	create_info.addressModeU = create_info.addressModeV = create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	create_info.anisotropyEnable = VK_TRUE;
	create_info.maxAnisotropy = 1.f;// Context::device.properties_2.properties.limits.maxSamplerAnisotropy;
	create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	create_info.unnormalizedCoordinates = VK_FALSE;
	create_info.compareEnable = VK_FALSE;
	create_info.compareOp = VK_COMPARE_OP_ALWAYS;
	create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

void Sampler::init(void)
{
	if (vkCreateSampler(Context::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create sampler");
}

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

void Image::init_compressed(vulkan::CommandBuffer &co, void* data, uint32_t size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags properties)
{
	int x, y, c;
	auto image_data = stbi_load_from_memory((uint8_t*)data, size, &x, &y, &c, STBI_rgb_alpha);
	
	init({ (uint32_t)x, (uint32_t)y }, format, usage, aspect, properties);
	
	vulkan::Buffer stagin_buffer;
	stagin_buffer.init(
		x * y * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	stagin_buffer.map();
	stagin_buffer.memcpy(image_data, x * y * 4);
	stagin_buffer.unmap();

	co.begin();

	barrier(co, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		(uint32_t)x,
		(uint32_t)y,
		1
	};

	vkCmdCopyBufferToImage(co.ptr, stagin_buffer.ptr, image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);

	co.end();
	vulkan::Fence f;
	f.init(false);
	co.submit_p(nullptr, nullptr, &f);
	f.wait();


	
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
	allocInfo.memoryTypeIndex = Context::device.find_memory_type(memRequirements.memoryTypeBits, properties);

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