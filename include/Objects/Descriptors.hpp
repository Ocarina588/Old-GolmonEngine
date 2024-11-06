#pragma once
#include "Context/Context.hpp"

namespace vulkan {

	class DescriptorPool {
	public:
		DescriptorPool(void);
		~DescriptorPool(void);

		void init(void);
		inline DescriptorPool& add_set(uint32_t size) { bindings.push_back({}); sizes.push_back(size); return *this; }
		inline DescriptorPool& add_binding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage, uint32_t count = 1) {
			bindings.rbegin()->emplace_back(binding, type, count, stage, nullptr);
			return *this;
		}
		inline VkDescriptorSet &get_set(uint32_t set, uint32_t index) {
			uint32_t offset = 0;
			for (uint32_t i = 0; i < set; i++)
				offset += sizes[i];
			return sets[offset + index];
		}
		void add_write(uint32_t set, uint32_t index, uint32_t binding_index, VkBuffer buffer);

		inline void write(void) { vkUpdateDescriptorSets(Context::device.ptr, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr); };

		std::vector<uint32_t> sizes;
		std::vector<VkDescriptorSetLayout> layouts;
		std::vector<VkDescriptorSet> sets;
		std::vector<VkWriteDescriptorSet> writes;
	private:
		VkDescriptorPool ptr = nullptr;

		std::vector<VkDescriptorBufferInfo> buffers_info;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
	};


}