#include "Core.hpp"
#include "chrono"
#include <Windows.h>

using Vk = vulkan::Context;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

std::vector<Vertex> vertices = {
	{{0.0f, -0.5f, 1.f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}}
};

void Gizmo::init(void)
{
	//std::vector<Vertex> lines_v = {
//	{{0, 0, 0}, {1, 0, 0} },
//	{{1, 0, 0}, {1, 0, 0} },
//	{{0, 0, 0}, {0, 1, 0} },
//	{{0, 1, 0}, {0, 1, 0} },
//	{{0, 0, 0}, {0, 0, 1} },
//	{{0, 0, 1}, {0, 0, 1} }
//};
	float square_size = .2f, offset = 0.2f;;
	std::vector<Vertex> squares_v = {
		//X
		{{0.f, offset, offset}, {1.f, 0.f, 0.f}},
		{{0.f, offset + square_size, offset}, {1.f, 0.f, 0.f }},
		{{0.f, offset, offset + square_size}, {1.f, 0.f, 0.f}},
		{{0.f, offset + square_size, offset}, {1.f, 0.f, 0.f}},
		{{0.f, offset + square_size, offset + square_size}, {1.f, 0.f, 0.f}},
		{{0.f, offset, offset + square_size}, {1.f, 0.f, 0.f}},

		//Y
		{{offset, 0.f, offset}, {0.f, 1.f, 0.f}},
		{{offset + square_size, 0.f, offset}, {0.f, 1.f, 0.f}},
		{{offset, 0.f, offset + square_size}, {0.f, 1.f, 0.f}},
		{{offset + square_size, 0.f, offset}, {0.f, 1.f, 0.f}},
		{{offset + square_size, 0.f, offset + square_size}, {0.f, 1.f, 0.f}},
		{{offset, 0.f, offset + square_size}, {0.f, 1.f, 0.f}},

		//Z
		{{offset, offset, 0.f}, {0.f, 0.f, 1.f}},
		{{offset + square_size, offset, 0.f}, {0.f, 0.f, 1.f}},
		{{offset, offset + square_size, 0.f}, {0.f, 0.f, 1.f}},
		{{offset + square_size, offset, 0.f}, {0.f, 0.f, 1.f}},
		{{offset + square_size, offset + square_size, 0.f}, {0.f, 0.f, 1.f}},
		{{offset, offset + square_size, 0.f}, {0.f, 0.f, 1.f}},


	};


	//lines.init(
	//	lines_v.data(),
	//	sizeof(Vertex) * lines_v.size(),
	//	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	//);
	squares.init(
		squares_v.data(),
		static_cast<uint32_t>(sizeof(Vertex) * squares_v.size()),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	//model = glm::mat4(1.f);

	////PIPELINE
	//vulkan::Shader vertex("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	//vulkan::Shader fragment("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	//pipeline.add_shader_stage(vertex.stage);
	//pipeline.add_shader_stage(fragment.stage);
	//pipeline.set_render_pass(render_pass);
	//pipeline.add_binding(Vertex::getBindingDescription());
	//for (auto i : Vertex::getAttributeDescriptions())
	//	pipeline.add_attribute(i);
	//pipeline.add_layout(layout);

	//pipeline.init();
}

Core::Core(void)
{
	rendering_mode = true;;;

	init_engine_resources();
	init_app_resources();

	glfwSetWindowUserPointer(Vk::window.ptr, this);
	glfwSetKeyCallback(Vk::window.ptr, keyCallback);
	glfwSetCursorPosCallback(Vk::window.ptr, mouse_callback);
	glfwSetMouseButtonCallback(Vk::window.ptr, mouse_button_callback);
	glfwSetScrollCallback(Vk::window.ptr, scroll_callback);

	init_imgui();

	//scene.load_obj("models/cube.obj");
} 

Core::~Core(void)
{

}

void Core::init_engine_resources(void)
{
	Vk::window.init(600, 600, "Vulkan App");
	Vk::instance.add_layer("VK_LAYER_LUNARG_monitor");

	context.init(true);

	finished_rendering.init();
	image_acquired.init();
	inflight.init(true);
	command_pool.init(Vk::device.index.graphics);
	command_buffer.init(command_pool);

	depth_image.init(
		context.window.extent,
		VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	offscreen_image.init(
		context.window.extent,
		Vk::window.format.format , VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	render_pass.use_depth(depth_image); 

	scene.load_obj("box.obj");
	scene.load_obj("box.obj");
	//scene.load_obj("models/CornellBox-Original.obj");

}

void Core::init_app_resources(void)
{
	//RENDER PASS
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	render_pass.init();
	Vk::window.init_framebuffers(render_pass);

	VkImageView views[] = { offscreen_image.view, depth_image.view };
	VkFramebufferCreateInfo create_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	create_info.attachmentCount = 2;
	create_info.pAttachments = views;
	create_info.width = Vk::window.extent.width;
	create_info.height = Vk::window.extent.height;
	create_info.renderPass = render_pass.ptr;
	create_info.layers = 1;
	
	if (vkCreateFramebuffer(Vk::device.ptr, &create_info, nullptr, &offscreen_buffer) != VK_SUCCESS) throw std::runtime_error("failed to create framebuffer");

	//BUFFERS
	vertex_buffer.init(
		(void*)vertices.data(),
		static_cast<uint32_t>(sizeof(Vertex) * vertices.size()),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	staging_buffer.init(
		sizeof(uint32_t) * Vk::window.extent.width * Vk::window.extent.height,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	staging_buffer.map();
	std::memset(staging_buffer.data, 0, staging_buffer.size);

	ubo_buffer.init(
		static_cast<uint32_t>(sizeof(UBO)),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	ubo_buffer.map();
	camera_raytracing.udpate_raytracing({0.f, 0.f, 4.f});
	camera_raytracing.init(Vk::window.extent.width, Vk::window.extent.height, &scene);


	//DESCRIPTORS
	descriptors.add_set(2)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	descriptors.init();

	descriptors.add_write(0, 0, 0, ubo_buffer.ptr);
	descriptors.add_write(0, 1, 0, camera_raytracing.ubo.ptr);
	descriptors.write();;

	//PIPELINE
	vulkan::Shader vertex("shaders/vertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
	vulkan::Shader fragment("shaders/fragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	pipeline.add_shader_stage(vertex.stage);
	pipeline.add_shader_stage(fragment.stage);
	pipeline.set_render_pass(render_pass);
	pipeline.add_binding(Vertex::getBindingDescription());
	for (auto i : Vertex::getAttributeDescriptions())
		pipeline.add_attribute(i);
	pipeline.add_layout(descriptors.layouts[0]);

	pipeline.init();

	gizmo.pipeline = pipeline;
	gizmo.pipeline.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	gizmo.pipeline.init();
	gizmo.init();



}

void Core::init_imgui(void)
{
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(Vk::window.ptr, true);

	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	if (vkCreateDescriptorPool(Vk::device.ptr, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) throw std::runtime_error("failed to create descriptor pool");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Vk::instance.ptr;
	init_info.PhysicalDevice = Vk::device.physical_ptr;
	init_info.Device = Vk::device.ptr;
	init_info.Queue = Vk::device.queue.graphics;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = Vk::window.surface_capabilities.minImageCount;
	init_info.ImageCount = Vk::window.surface_capabilities.minImageCount + 1;
	init_info.RenderPass = render_pass.ptr;
	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();
}

#include <windows.h>
#include <iostream>
#include <string>

#include <windows.h>
#include <iostream>
#include <string>

HANDLE SetupFileChangeNotification(const std::wstring& directory) {
	HANDLE hChangeHandle = FindFirstChangeNotification(
		directory.c_str(),
		TRUE,                           // Do not watch subdirectories
		FILE_NOTIFY_CHANGE_LAST_WRITE);  // Watch for file modifications

	if (hChangeHandle == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to set up directory change notification." << std::endl;
	}

	return hChangeHandle;
}

bool CheckFileTimestampChanged(const std::wstring& filepath, FILETIME& lastWriteTime) {
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if (GetFileAttributesEx(filepath.c_str(), GetFileExInfoStandard, &fileInfo)) {
		// Check if the last write time has changed
		if (CompareFileTime(&fileInfo.ftLastWriteTime, &lastWriteTime) != 0) {
			// Update last write time to the latest time
			lastWriteTime = fileInfo.ftLastWriteTime;
			return true;  // File has been modified
		}
	}
	return false;  // No change in file timestamp
}

bool CheckFileChanged(HANDLE hChangeHandle, const std::wstring& filepath, FILETIME& lastWriteTime) {
	DWORD waitStatus = WaitForSingleObject(hChangeHandle, 0);  // Non-blocking wait

	if (waitStatus == WAIT_OBJECT_0) {
		// A change in the directory was detected, check the specific file's timestamp
		bool fileChanged = CheckFileTimestampChanged(filepath, lastWriteTime);

		// Reset the directory change notification for future monitoring
		FindNextChangeNotification(hChangeHandle);

		return fileChanged;
	}

	return false;
}

int Core::main(int ac, char** av)
{
	HANDLE hChangeHandle = SetupFileChangeNotification(L"./");
	if (hChangeHandle == INVALID_HANDLE_VALUE) {
		return 1;  // Exit if unable to set up change notification
	}
	bool tmp = false;
	// Get the initial last write time of the file
	FILETIME lastWriteTime = { 0 };

	while (Vk::window.should_close() == false) {
		update_dt();
		camera.update(dt);
		//camera_raytracing.udpate_raytracing(camera.pos);

		if (tmp == false && file_loaded.empty() == false) {
			tmp = true;
			CheckFileTimestampChanged(file_loaded, lastWriteTime);
		}

		Vk::window.poll_events();;
		inflight.wait();
		Vk::window.acquire_next_image(image_acquired.ptr, nullptr);
		
		if (CheckFileChanged(hChangeHandle, file_loaded, lastWriteTime)) {
			std::wcout << L"File modified: " << "filename" << std::endl;
			scene.load_obj(file_loaded_n.c_str());
		}

		update_ubo();

		if (rendering_mode)
			render_gpu();
		else 
			render_cpu();

		command_buffer.submit(image_acquired, finished_rendering, inflight);
		
		Vk::window.present(finished_rendering);
	}

	return 0;
}

void Core::update_dt(void)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
}

void Core::update_ubo(void)
{
	UBO ubo{};
	ubo.model = glm::mat4(1.f);// glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//ubo.model = glm::rotate(ubo.model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	//ubo.model = glm::rotate(ubo.model, dt * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));


	ubo.view = camera.view;
	//ubo.view = glm::lookAt(glm::vec3(0.0f, -5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.proj = glm::perspective(glm::radians(45.0f), Vk::window.extent.width / (float)Vk::window.extent.height, 0.1f, 100.0f);

	ubo.proj[1][1] *= -1;


	ubo_buffer.memcpy(&ubo, sizeof(ubo));

	ubo.model = camera_raytracing.model;

	camera_raytracing.ubo.memcpy(&ubo, sizeof(ubo));
}

std::string ask_file(char const* filter)
{
	OPENFILENAMEA ofn;
	char filename[256] = { 0 };
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 256;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "";

	std::string file;

	if (GetOpenFileNameA(&ofn))
		file = filename;
	return file;
}

#include <string>
#include <locale>
#include <codecvt>

std::wstring StringToWString(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
	std::wstring wstr(size_needed, L'\0'); // Create a wstring of the required size
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size_needed);
	return wstr;
}

void Core::render_gpu(void)
{
	static std::string file_to_load = "";

	if (file_to_load != "") {
		scene.load_obj(file_to_load.c_str());
		file_loaded = StringToWString(file_to_load);
		file_loaded_n = file_to_load;
	}
	file_to_load = "";

	
	command_buffer.begin(true);
	{
		render_pass.begin(command_buffer, Vk::window.extent, offscreen_buffer);

		pipeline.bind(command_buffer);

		vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);
		scene.draw(command_buffer); 
		
		//VkDeviceSize offset = 0;
		//vkCmdBindVertexBuffers(command_buffer.ptr, 0, 1, &vertex_buffer.ptr, &offset);
		//vkCmdDraw(command_buffer.ptr, 3, 1, 0, 0);
	
		gizmo.pipeline.bind(command_buffer);

		vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, gizmo.pipeline.layout, 0, 1, &descriptors.get_set(0, 0), 0, nullptr);
		gizmo.draw(command_buffer);

		vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, gizmo.pipeline.layout, 0, 1, &descriptors.get_set(0, 1), 0, nullptr);
		camera_raytracing.draw(command_buffer);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();


		if (ImGui::BeginMainMenuBar()) {


			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open file")) file_to_load = ask_file("Obj files (*.obj)\0*.obj\0").c_str();
				ImGui::EndMenu();
			}

			//if (ImGui::BeginMenu("Project")) {
			//	if (ImGui::MenuItem("Terrain")) {
			//		terrain = std::make_shared<engine::Terrain>(std::make_shared<engine::Perlin>(std::time(NULL)));
			//		mode = true;
			//	}
			//	if (ImGui::MenuItem("Rubik's Cube"))
			//		mode = false;
			//	ImGui::EndMenu();

			//}

			if (ImGui::BeginMenu("Camera")) {
				if (ImGui::MenuItem("Scene"));
				if (ImGui::MenuItem("Player"));
				ImGui::EndMenu();
			}


			ImGui::EndMainMenuBar();
		}


		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.ptr);

		render_pass.end(command_buffer);

		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		vulkan::Image::cpy(command_buffer, Vk::window.extent, offscreen_image.image, Vk::window.images[Vk::window.image_index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);
	}
	command_buffer.end();
}

void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0; // No row length
	region.bufferImageHeight = 0; // No image height
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0; // Base mip level
	region.imageSubresource.baseArrayLayer = 0; // First layer
	region.imageSubresource.layerCount = 1; // One layer
	region.imageOffset = { 0, 0, 0 }; // Copy to the top-left corner
	region.imageExtent = { width, height, 1 }; // Width, height, and depth

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

#include <thread>

void Core::render_cpu(void)
{
	//static std::thread l([&] {
	//	while (true) cpu_raytracing();
	//});

	cpu_raytracing();

	command_buffer.begin();
	{
		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);

		copyBufferToImage(command_buffer.ptr, 
			staging_buffer.ptr, Vk::window.images[Vk::window.image_index],
			Vk::window.extent.width, Vk::window.extent.height
		);

		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);
	}
	command_buffer.end();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Core& core = *(Core *)glfwGetWindowUserPointer(Vk::window.ptr);

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_TAB:
			core.rendering_mode = !core.rendering_mode;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static double old_xpos = 0;
	static double old_ypos = 0;
	Core& core = *(Core*)glfwGetWindowUserPointer(Vk::window.ptr);
	if (core.clicked)
		core.camera.process_mouse(core.dt, xpos - old_xpos, ypos - old_ypos);
	old_xpos = xpos;
	old_ypos = ypos;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	Core& core = *(Core*)glfwGetWindowUserPointer(Vk::window.ptr);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		core.clicked = true;
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		core.clicked = false;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	Core& core = *(Core*)glfwGetWindowUserPointer(Vk::window.ptr);

	core.camera.process_scroll(core.dt, xoffset, yoffset);
}

void Core::create_gizmo(void)
{



}

#include <omp.h>

#define BEGIN_LOOP _Pragma("omp parallel for collapse(2)") for (int y = 0; y < height; y++) { for (int x = 0; x < width; x++) { uint32_t index = y * width + x;
#define END_LOOP }}

struct pixel_s {
	unsigned char b = 0;
	unsigned char g = 0;
	unsigned char r = 0;
	unsigned char a = 255;
};

static bool intersectTriangle(
	Ray &ray,
	const glm::vec3& v0,    // First vertex of the triangle
	const glm::vec3& v1,    // Second vertex of the triangle
	const glm::vec3& v2,    // Third vertex of the triangle
	float& t,               // Distance to intersection point (output)
	glm::vec3& intersectionPoint) // Intersection point (output)
{
	const float EPSILON = 1e-8;
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 h = glm::cross(ray.dir, edge2);
	float a = glm::dot(edge1, h);

	if (a > -EPSILON && a < EPSILON) {
		return false; // Ray is parallel to the triangle
	}

	float f = 1.0f / a;
	glm::vec3 s = ray.orig - v0;
	float u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f) {
		return false; // Intersection is outside the triangle
	}

	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(ray.dir, q);

	if (v < 0.0f || u + v > 1.0f) {
		return false; // Intersection is outside the triangle
	}

	t = f * glm::dot(edge2, q);

	if (t > EPSILON) { // Ray intersection
		intersectionPoint = ray.orig + ray.dir * t;
		return true;
	}
	else {
		return false; // Intersection is behind the ray origin
	}
}

glm::vec3 Core::trace_ray(Ray r, int max_bounce)
{
	glm::vec3 light{};
	glm::vec3 final_color{1.f, 1.f, 1.f};
	bool nothing = true;
	int direct_light = 0;
	for (int i = 0; i <= max_bounce; i++) {
		hit_info info;
		bool found = false;

		for (auto& mesh : scene.meshes) {
			hit_info tmp;
			if (!mesh.hit(r, tmp)) continue;
			found = true;
			nothing = false;

			if (tmp.distance < info.distance)
				info = tmp;
		}

		if (!found) break;

		light.x += final_color.x * info.material.Ks.X * 3.f;;
		light.y += final_color.y * info.material.Ks.Y * 3.f;;
		light.z += final_color.z * info.material.Ks.Z * 3.f;;

		final_color.x *= info.material.Kd.X;
		final_color.y *= info.material.Kd.Y;
		final_color.z *= info.material.Kd.Z;

		//final_color = info.normal;

		r.orig = info.pos;
		r.dir = Camera::generateRandomDirection(info.normal);
	}

	if (nothing) return {};

	//return final_color;

	return light;
}

void Core::cpu_raytracing(void)
{
	int num_rays = 200;
	int max_bounce = 10;
	float light_intensity = 40.f;

	int width = (int)Vk::window.extent.width;
	int height = (int)Vk::window.extent.height;

	pixel_s* screen = (pixel_s *)staging_buffer.data;

	auto start_point = camera_raytracing.top_left + (camera_raytracing.du + camera_raytracing.dv) / 2.f;
	static int num_frames = 0;

	BEGIN_LOOP(width, height);

		Ray new_ray(camera_raytracing.pos, start_point + camera_raytracing.du * (float)x + camera_raytracing.dv * (float)y);
		//screen[index] = {};
		glm::vec3 light = {};

		for (int i = 0; i < num_rays ; i++)
			light += trace_ray(new_ray, max_bounce);

		glm::vec3 color{ light.x , light.y, light.z};
		color /= (float)num_rays;
		color *= 255.f * light_intensity;
		float weight = 1.f / (num_frames + 1);

		if (num_frames == 0) {
			screen[index].r = (char)std::clamp(color.x, 0.f, 255.f);
			screen[index].g = (char)std::clamp(color.y, 0.f, 255.f);
			screen[index].b = (char)std::clamp(color.z, 0.f, 255.f);
		}
		else {
			screen[index].r = screen[index].r * (1.f - weight) + (char)std::clamp(color.x, 0.f, 255.f) * weight;
			screen[index].g = screen[index].g * (1.f - weight) + (char)std::clamp(color.y, 0.f, 255.f) * weight;
			screen[index].b = screen[index].b * (1.f - weight) + (char)std::clamp(color.z, 0.f, 255.f) * weight;
		}

	END_LOOP;

	num_frames;
}
