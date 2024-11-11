#include <unordered_map>
#include "Objects/Descriptors.hpp"

using namespace vulkan;

DescriptorPool::DescriptorPool(void)
{

}

DescriptorPool::~DescriptorPool(void)
{
	for (auto i : layouts)
		vkDestroyDescriptorSetLayout(Context::device.ptr, i, nullptr);
	vkDestroyDescriptorPool(Context::device.ptr, ptr, nullptr);
}

void DescriptorPool::init(void)
{
	VkDescriptorPoolCreateInfo create_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO }; 
	std::unordered_map<VkDescriptorType, uint32_t> m;
	std::vector<VkDescriptorPoolSize> pool_size;
	uint32_t size = 0;

	layouts.resize(bindings.size());

	for (int i = 0; i < bindings.size(); i++) {
		VkDescriptorSetLayoutCreateInfo layout_create_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layout_create_info.bindingCount = static_cast<uint32_t>(bindings[i].size());
		layout_create_info.pBindings = bindings[i].data(); 
		if (vkCreateDescriptorSetLayout(Context::device.ptr, &layout_create_info, nullptr, &layouts[i]) != VK_SUCCESS) throw std::runtime_error("failed to create descriptor set");

		for (auto j : bindings[i])
			m[j.descriptorType] += sizes[i];

		size += sizes[i];
	}

	std::vector<VkDescriptorSetLayout> layouts_ref;
	layouts_ref.reserve(size);

	for (int i = 0; i < sizes.size() ; i++)
		for (uint32_t j = 0; j < sizes[i]; j++)
			layouts_ref.push_back(layouts[i]);

	for (auto i : m)
		pool_size.emplace_back(i.first, i.second);

	create_info.maxSets = static_cast<uint32_t>(size);
	create_info.poolSizeCount = static_cast<uint32_t>(pool_size.size());;
	create_info.pPoolSizes = pool_size.data();

	if (vkCreateDescriptorPool(Context::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) throw std::runtime_error("failed to create descriptor pool");

	VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	alloc_info.descriptorPool = ptr;
	alloc_info.descriptorSetCount = size;
	alloc_info.pSetLayouts = layouts_ref.data();

	sets.resize(size);

	if (vkAllocateDescriptorSets(Context::device.ptr, &alloc_info, sets.data()) != VK_SUCCESS) throw std::runtime_error("failed to allocate descriptor set");

	buffers_info.reserve(size);
}

void DescriptorPool::add_write(uint32_t set, uint32_t index, uint32_t binding_index, VkBuffer buffer)
{
	buffers_info.emplace_back(buffer, 0, VK_WHOLE_SIZE);
	
	VkWriteDescriptorSet ws{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	ws.dstSet = get_set(set, index);
	ws.dstBinding = bindings[set][binding_index].binding;
	ws.descriptorCount = bindings[set][binding_index].descriptorCount;
	ws.descriptorType = bindings[set][binding_index].descriptorType;
	ws.pBufferInfo = &buffers_info[buffers_info.size() - 1];

	writes.push_back(ws);
}

void DescriptorPool::write(void)
{
	vkUpdateDescriptorSets(Context::device.ptr, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}