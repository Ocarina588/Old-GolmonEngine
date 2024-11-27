#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

namespace vulkan {
	class RenderPass;
	class Semaphore;
	class Fence;

	class Window {
	public:
		friend class Context;

		void init(int w, int h, char const* title);
		void init_framebuffers(RenderPass&);

		Window(void);
		~Window(void);

		std::vector<char const*> get_extensions(void);
		inline void poll_events(void) { glfwPollEvents(); }
		inline bool should_close(void) { return glfwWindowShouldClose(ptr); }
		void acquire_next_image(VkSemaphore s, VkFence f);

		void present(Semaphore& s);
		GLFWwindow* ptr = nullptr;

		uint32_t image_index = 0;
		VkSurfaceKHR surface = nullptr;
		VkSwapchainKHR swapchain = nullptr;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		std::vector<VkFramebuffer> framebuffers;
		VkExtent2D extent{};
		VkSurfaceFormatKHR format;
		VkSurfaceCapabilitiesKHR surface_capabilities;

	private:

		void init_surface(void);
		void init_swapchain(void);

		void create_resources(void);
		bool created = false;
	};
}