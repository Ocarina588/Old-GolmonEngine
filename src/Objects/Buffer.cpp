#include "Objects/Buffer.hpp"

using namespace vulkan;

Buffer::Buffer(uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	init(_size, usage, properties);
}

void Buffer::init(uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	VkBufferCreateInfo create_info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	create_info.size = _size;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.usage = usage;

	if (vkCreateBuffer(Context::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) throw std::runtime_error("failed to create buffer");

	auto requirements = Context::device.get_memory_requirements(ptr);

	VkMemoryAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = Context::device.find_memory_type(requirements.memoryTypeBits, properties);

	if (vkAllocateMemory(Context::device.ptr, &alloc_info, nullptr, &memory) != VK_SUCCESS) throw std::runtime_error("failed to alloc memory");

	size = requirements.size;

	vkBindBufferMemory(Context::device.ptr, ptr, memory, 0);
}

Buffer::~Buffer(void)
{
	vkFreeMemory(Context::device.ptr, memory, nullptr);
	vkDestroyBuffer(Context::device.ptr, ptr, nullptr);
}