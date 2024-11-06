#pragma once

#include <imconfig.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "Golmon.hpp"
#include "Model.hpp"
#include <array>

class Core {
public:
	Core(void);
	~Core(void);

	int main(int ac, char** av);
	void init_engine_resources(void);
	void init_app_resources(void);
	void present_image(void);

	void render_gpu(void);
	void render_cpu(void);
	void cpu_raytracing(void);

	void draw(void);
	void update_ubo(void);

	void init_imgui(void);

	vulkan::Context context;

	vulkan::Buffer vertex_buffer, ubo_buffer, staging_buffer;
	vulkan::Image offscreen_image, depth_image;
	VkFramebuffer offscreen_buffer;
	vulkan::RenderPass render_pass;
	vulkan::GraphicsPipeline pipeline;
	vulkan::DescriptorPool descriptors;
	VkDescriptorPool imguiPool = nullptr;
	vulkan::CommandPool command_pool;
	vulkan::CommandBuffer command_buffer;
	vulkan::Semaphore image_acquired, finished_rendering;
	vulkan::Fence inflight;

	std::wstring file_loaded = L"";
	std::string file_loaded_n = "";
	bool rendering_mode = true;

	Scene scene;
};