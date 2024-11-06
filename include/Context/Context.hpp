#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include "Instance.hpp"
#include "Device.hpp"
#include "Window.hpp"

namespace vulkan{


	class Context {
	public:
		Context(void);
		~Context(void);

		void init(bool debug = false);

		static Instance instance;
		static Device device;
		static Window window;

	private:
		bool debug = false;
	};

}