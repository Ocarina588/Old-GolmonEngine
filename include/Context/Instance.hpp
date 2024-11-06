#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

namespace vulkan {

	class Instance {
	public:
		friend class Context;

		Instance(void);
		~Instance(void);

		inline void add_extension(char const* str) { extensions.push_back(str); }
		inline void add_layer(char const* str) { layers.push_back(str); }

	private:

		void init(void);

		void create_messenger(void);
		void destroy_messenger(void);
	public:
		VkInstance ptr = nullptr;
	private:
		std::vector<char const*> extensions;
		std::vector<char const*> layers;

		VkDebugUtilsMessengerEXT messenger = nullptr;
	};
}