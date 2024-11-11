#include <assert.h>
#include "Golmon.hpp"

using namespace vulkan;

Device::Device(void)
{

}

Device::~Device(void)
{
	vkDestroyDevice(ptr, nullptr);
}

void Device::init(VkInstance instance, VkSurfaceKHR surface)
{
	VkDeviceCreateInfo create_info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(count);
	vkEnumeratePhysicalDevices(instance, &count, physical_devices.data());

	physical_ptr = physical_devices[gpu_index];

	get_queues(surface);

	float priority = 1.f;
	VkDeviceQueueCreateInfo queue_create_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queue_create_info.queueCount = 1;
	queue_create_info.queueFamilyIndex = index.graphics;
	queue_create_info.pQueuePriorities = &priority;

	VkPhysicalDeviceFeatures features{};
	features.fillModeNonSolid = true;
	features.wideLines = true;

	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();
	create_info.pEnabledFeatures = &features;
	create_info.pQueueCreateInfos = &queue_create_info;
	create_info.queueCreateInfoCount = 1;
	
	if (vkCreateDevice(physical_ptr, &create_info, nullptr, &ptr) != VK_SUCCESS) throw std::runtime_error("failed to create device");

	vkGetDeviceQueue(ptr, index.graphics, 0, &queue.graphics);
	vkGetDeviceQueue(ptr, index.compute, 0, &queue.compute);
	vkGetDeviceQueue(ptr, index.transfer, 0, &queue.transfer);

	if (surface)
		vkGetDeviceQueue(ptr, index.present, 0, &queue.present);

}

void Device::get_queues(VkSurfaceKHR surface)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_ptr, &count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_properties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_ptr, &count, queue_properties.data());

	for (uint32_t i = 0; i < queue_properties.size(); i++) {
		if (index.graphics == ~0 && queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			index.graphics = i;
		if (index.compute == ~0u && queue_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			index.compute = i;
		if (index.transfer == ~0u && queue_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			index.transfer = i;

		if (surface == nullptr) continue;

		VkBool32 supported = false;
		if (vkGetPhysicalDeviceSurfaceSupportKHR(physical_ptr, i, surface, &supported) != VK_SUCCESS) throw std::runtime_error("get physical device surface support");
		if (index.present == ~0u && supported)
			index.present = i;
	}

	if (!(index.graphics == index.compute && index.graphics == index.transfer && index.compute == index.transfer)) throw std::runtime_error("wrong queues");
	if (surface)
		if (!(index.graphics == index.present)) throw std::runtime_error("wrong queues");
}

VkMemoryRequirements Device::get_memory_requirements(VkImage image)
{
	VkMemoryRequirements i;
	vkGetImageMemoryRequirements(Context::device.ptr, image, &i);
	return i;
}

VkMemoryRequirements Device::get_memory_requirements(VkBuffer buffer)
{
	VkMemoryRequirements i;
	vkGetBufferMemoryRequirements(Context::device.ptr, buffer, &i);
	return i;
}

uint32_t Device::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Context::device.physical_ptr, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	throw std::runtime_error("no memory type");
	return 0;
}