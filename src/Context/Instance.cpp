#include "Context/Instance.hpp"
#include "utils.hpp"
#include <assert.h>

using namespace vulkan;

static VkBool32 messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cout << TERMINAL_COLOR_CYAN << "Validation Layers: ";

	switch (messageSeverity) {
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT):
		std::cout << TERMINAL_COLOR_YELLOW << std::endl; break;
	case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT):
		std::cout << TERMINAL_COLOR_RED << std::endl; break;
	}

	std::cout << pCallbackData->pMessage << TERMINAL_COLOR_RESET << std::endl;

	return VK_SUCCESS;
}

static VkDebugUtilsMessengerCreateInfoEXT get_messenger_create_info(void)
{
	VkDebugUtilsMessengerCreateInfoEXT create_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	create_info.messageType = 0b1111;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;// | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	create_info.pfnUserCallback = messenger_callback;

	return create_info;
}

Instance::Instance(void)
{

}

Instance::~Instance(void)
{
	if (messenger)
		destroy_messenger();
	vkDestroyInstance(ptr, nullptr);
}

void Instance::init(void)
{
	VkInstanceCreateInfo create_info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	auto messenger_create_info = get_messenger_create_info();
	bool debug = std::find(extensions.begin(), extensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != extensions.end();

	VkApplicationInfo app_info{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	app_info.pApplicationName = "GolmonEngine";
	app_info.apiVersion = VK_MAKE_VERSION(1, 3, 0);
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "GolmonEngine";

	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();
	create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
	create_info.ppEnabledLayerNames = layers.data();
	create_info.pApplicationInfo = &app_info;

	if (debug)
		create_info.pNext = &messenger_create_info;

	if (vkCreateInstance(&create_info, nullptr, &ptr) != VK_SUCCESS) throw std::runtime_error("failed to create instance");

	if (debug)
		create_messenger();
}

void Instance::create_messenger(void)
{
	auto create_info = get_messenger_create_info();
	auto func = GET_INSTANCE_PROC(ptr, PFN_vkCreateDebugUtilsMessengerEXT);
	
	if (!func) throw std::runtime_error("failed to get instance proc");

	if (func(ptr, &create_info, nullptr, &messenger) != VK_SUCCESS) throw std::runtime_error("failed to create messenger");
}

void Instance::destroy_messenger(void)
{
	auto func GET_INSTANCE_PROC(ptr, PFN_vkDestroyDebugUtilsMessengerEXT);

	if (!func) throw std::runtime_error("faild to get instance proc");
	func(ptr, messenger, nullptr);
}