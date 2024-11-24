#pragma once
#include "Context/Context.hpp"

namespace vulkan {
	class Semaphore;
	class Fence;

	class CommandPool {
	public:
		CommandPool(uint32_t index) { init(index); }
		CommandPool(void) {}
		~CommandPool(void);

		void init(uint32_t index);

		VkCommandPool ptr = nullptr;
	private:
	};

	class CommandBuffer {
	public:
		CommandBuffer(CommandPool& a) { init(a); }
		CommandBuffer(void) {};
		~CommandBuffer(void);
		void init(CommandPool&);
		void begin(bool _reset = true);
		void end(void);
		void submit(Semaphore& wait, Semaphore& signal, Fence& inflight);
		void submit_p(Semaphore *wait, Semaphore *signal, Fence *inflight);
		inline void reset(void) { vkResetCommandBuffer(ptr, 0); }


		VkCommandBuffer ptr = nullptr;
		VkCommandPool pool = nullptr;
	private:
	};
}