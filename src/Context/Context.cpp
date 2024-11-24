#include "Golmon.hpp"

using namespace vulkan;

Instance Context::instance;
Device Context::device;
Window Context::window;

Context::Context(void)
{

}

Context::~Context(void)
{

}

void Context::init(bool debug)
{
	if (window.created) {
		for (auto i : window.get_extensions())
			instance.add_extension(i);
		device.add_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	if (debug) {
		instance.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instance.add_layer("VK_LAYER_KHRONOS_validation");
	}

	instance.init();

	if (window.created)
		window.init_surface();

	device.init(instance.ptr, window.surface);

	if (window.created)
		window.init_swapchain();
}