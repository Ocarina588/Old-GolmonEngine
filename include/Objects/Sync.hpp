#pragma once
#include "Context/Context.hpp"

namespace vulkan {

	class Semaphore {
	public:
		Semaphore(void) {};
		void init(void) {
			VkSemaphoreCreateInfo create_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			if (vkCreateSemaphore(Context::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) throw std::runtime_error("faield to create semaphore");
		}
		~Semaphore(void) {
			vkDestroySemaphore(Context::device.ptr, ptr, nullptr);
		}

		VkSemaphore ptr = nullptr;
	private:
	};

	class Fence {
	public:
		Fence(void) {}
		void init(bool signaled = true) {
			VkFenceCreateInfo create_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

			if (vkCreateFence(Context::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) throw std::runtime_error("failed to create fence");
		}
		~Fence(void) {
			vkDestroyFence(Context::device.ptr, ptr, nullptr);
		}

		inline void wait(bool _reset = true) { if (vkWaitForFences(Context::device.ptr, 1, &ptr, VK_TRUE, UINT64_MAX) != VK_SUCCESS) throw std::runtime_error("failed to wait for fences"); if (_reset) reset(); }
		inline void reset(void) { vkResetFences(Context::device.ptr, 1, &ptr); }

		VkFence ptr = nullptr;
	private:
	};
}