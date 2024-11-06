#pragma once
#include "Context/Context.hpp"

namespace vulkan{

	class Buffer {
	public:
		Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		Buffer(void* p, uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) { init(p, _size, usage, properties); }
		Buffer(void) {};
		~Buffer(void);

		void init(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		inline void init(void* p, uint32_t _size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
			init(_size, usage, properties);
			map();
			memcpy(p, _size);
			unmap();
		}

		inline void map(void) { if (vkMapMemory(Context::device.ptr, memory, 0, size, 0, &data) != VK_SUCCESS) throw std::runtime_error("failed to map memory"); }
		inline void unmap(void) { vkUnmapMemory(Context::device.ptr, memory); }
		inline void memcpy(void* p, uint32_t _size) { std::memcpy(data, p, _size); }

		void* data = nullptr;

		VkBuffer ptr = nullptr;
		VkDeviceMemory memory = nullptr;
		VkDeviceAddress adress = 0;
		VkDeviceSize size = 0;
	private:
	};

}