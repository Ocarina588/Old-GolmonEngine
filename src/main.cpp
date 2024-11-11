//#include <iostream>
//#include <array>
//#include "Golmon.hpp"
//#include <chrono>
//
//struct Vertex {
//	glm::vec2 pos;
//	glm::vec3 color;
//
//	static VkVertexInputBindingDescription getBindingDescription() {
//		VkVertexInputBindingDescription bindingDescription{};
//		bindingDescription.binding = 0;
//		bindingDescription.stride = sizeof(Vertex);
//		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//		return bindingDescription;
//	}
//
//	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
//		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
//
//		attributeDescriptions[0].binding = 0;
//		attributeDescriptions[0].location = 0;
//		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
//		attributeDescriptions[0].offset = offsetof(Vertex, pos);
//
//		attributeDescriptions[1].binding = 0;
//		attributeDescriptions[1].location = 1;
//		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//		attributeDescriptions[1].offset = offsetof(Vertex, color);
//
//		return attributeDescriptions;
//	}
//};
//
//const std::vector<Vertex> vertices = {
//	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
//};
//
//struct UBO {
//	glm::mat4 model;
//	glm::mat4 view;
//	glm::mat4 proj;
//};
//
//using Vk = vulkan::Context;
//
//void update_ubo(vulkan::Buffer &ubo_buffer)
//{
//	static auto startTime = std::chrono::high_resolution_clock::now();
//
//	auto currentTime = std::chrono::high_resolution_clock::now();
//	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
//
//	UBO ubo{};
//	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//
//	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//
//	ubo.proj = glm::perspective(glm::radians(45.0f), Vk::window.extent.width / (float)Vk::window.extent.height, 0.1f, 10.0f);
//
//	ubo.proj[1][1] *= -1;
//
//	ubo_buffer.memcpy(&ubo, sizeof(ubo));
//}
//
//int main(int ac, char** av)
//{
//	vulkan::Context context;
//	Vk::window.init(1280, 720, "Vulkan App");
//	Vk::instance.add_layer("VK_LAYER_LUNARG_monitor");
//	context.init(true);
//
//	vulkan::Buffer vertex_buffer{
//	(void*)vertices.data(),
//	static_cast<uint32_t>(sizeof(Vertex) * vertices.size()),
//	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
//	};
//
//	vulkan::Buffer ubo_buffer{
//		static_cast<uint32_t>(sizeof(UBO)),
//		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
//	};
//	ubo_buffer.map();
//
//	vulkan::Image depth_image;
//	depth_image.init(
//		context.window.extent,
//		VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
//		VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
//	);
//
//	vulkan::RenderPass render_pass;
//	render_pass.use_depth(depth_image);
//	render_pass.set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
//	render_pass.init();
//
//	Vk::window.init_framebuffers(render_pass);
//
//	vulkan::DescriptorPool descriptor_pool;
//	descriptor_pool.add_set(1)
//		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
//	descriptor_pool.init();
//
//	descriptor_pool.add_write(0, 0, 0, ubo_buffer.ptr);
//	descriptor_pool.write();
//
//	vulkan::GraphicsPipeline pipeline;
//	{
//		vulkan::Shader vertex("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
//		vulkan::Shader fragment("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
//
//		pipeline.add_shader_stage(vertex.stage);
//		pipeline.add_shader_stage(fragment.stage);
//		pipeline.set_render_pass(render_pass);
//		pipeline.add_binding(Vertex::getBindingDescription()); 
//		for (auto i : Vertex::getAttributeDescriptions())
//			pipeline.add_attribute(i);
//		pipeline.add_layout(descriptor_pool.layouts[0]);
//			
//		pipeline.init();
//	}
//
//	vulkan::CommandPool command_pool(Vk::device.index.graphics);
//	vulkan::CommandBuffer command_buffer(command_pool);
//	vulkan::Semaphore image_acquired, finished_rendering;
//	vulkan::Fence inflight_image;
//
//
//	while (Vk::window.should_close() == false) {
//		Vk::window.poll_events();
//
//		inflight_image.wait();
//
//		uint32_t image_index = 0;
//		vkAcquireNextImageKHR(Vk::device.ptr, Vk::window.swapchain, UINT64_MAX, image_acquired.ptr, nullptr, &image_index);
//
//		command_buffer.begin(); 
//		{
//			render_pass.begin(command_buffer, Vk::window.extent, Vk::window.framebuffers[image_index]);
//			pipeline.bind(command_buffer);
//
//			update_ubo(ubo_buffer);
//
//			VkDeviceSize offset = 0;
//			vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_buffer.ptr, &offset);
//
//			vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptor_pool.get_set(0, 0), 0, nullptr);
//
//			vkCmdDraw(command_buffer.ptr, 3, 1, 0, 0);
//			render_pass.end(command_buffer);
//		}
//		command_buffer.end();
//
//		command_buffer.submit(image_acquired, finished_rendering, inflight_image);
//
//		VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
//		present_info.waitSemaphoreCount = 1;
//		present_info.pWaitSemaphores = &finished_rendering.ptr;
//		present_info.swapchainCount = 1;
//		present_info.pSwapchains = &Vk::window.swapchain;
//		present_info.pImageIndices = &image_index;
//
//		assert(vkQueuePresentKHR(Vk::device.queue.present, &present_info) == VK_SUCCESS);
//	}
//
//	vkDeviceWaitIdle(Vk::device.ptr);
//
//	return (0);
//}

#include "Core.hpp"

int main(int ac, char** av)
{
	try {
		Core core;

		return core.main(ac, av);
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
	}
	return 1;
}