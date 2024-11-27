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

		{{0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}},
		{{offset, 0.f, 0.f}, {1.f, 0.f, 0.f}},
		{{offset, 0.f, 0.f}, {1.f, 0.f, 0.f}},

		//Y
		{{offset, 0.f, offset}, {0.f, 1.f, 0.f}},
		{{offset + square_size, 0.f, offset}, {0.f, 1.f, 0.f}},
		{{offset, 0.f, offset + square_size}, {0.f, 1.f, 0.f}},
		{{offset + square_size, 0.f, offset}, {0.f, 1.f, 0.f}},
		{{offset + square_size, 0.f, offset + square_size}, {0.f, 1.f, 0.f}},
		{{offset, 0.f, offset + square_size}, {0.f, 1.f, 0.f}},

		{{0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}},
		{{0.f, offset, 0.f}, {0.f, 1.f, 0.f}},
		{{0.f, offset, 0.f}, {0.f, 1.f, 0.f}},


		//Z
		{{offset, offset, 0.f}, {0.f, 0.f, 1.f}},
		{{offset + square_size, offset, 0.f}, {0.f, 0.f, 1.f}},
		{{offset, offset + square_size, 0.f}, {0.f, 0.f, 1.f}},
		{{offset + square_size, offset, 0.f}, {0.f, 0.f, 1.f}},
		{{offset + square_size, offset + square_size, 0.f}, {0.f, 0.f, 1.f}},
		{{offset, offset + square_size, 0.f}, {0.f, 0.f, 1.f}},

		{{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}},
		{{0.f, 0.f, offset}, {0.f, 0.f, 1.f}},
		{{0.f, 0.f, offset}, {0.f, 0.f, 1.f}},



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
	rendering_mode = false;;;

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
	Vk::window.init(2000, 1500, "Vulkan App");
	Vk::instance.add_layer("VK_LAYER_LUNARG_monitor");

	Vk::device.as_feature.accelerationStructure = VK_TRUE;
	Vk::device.raytracing_pipeline_feature.rayTracingPipeline = VK_TRUE;
	Vk::device.device_address_feature.bufferDeviceAddress = VK_TRUE;
	Vk::device.add_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &Vk::device.raytracing_pipeline_feature);
	Vk::device.add_extension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &Vk::device.as_feature);
	Vk::device.add_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &Vk::device.device_address_feature);
	Vk::device.add_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	Vk::device.add_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	Vk::device.add_extension(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
	
	Vk::device.set_gpu(1);

	context.init(true);

	std::cout << Vk::device.properties_2.properties.deviceName << std::endl;
	std::cout << "maximum recursion: " << Vk::device.rtProps.maxRayRecursionDepth << std::endl;

	vulkan::RayTracingPipeline::load_raytracing_functions();

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
		Vk::window.format.format , 
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	//offscreen_image.barrier(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	render_pass.use_depth(depth_image); 

	scene.load_scene("stormtrooper.glb");
	std::cout << "loaded" << std::endl;
}

void Core::init_app_resources(void)
{
	//RENDER PASS
	render_pass.set_final_layout(VK_IMAGE_LAYOUT_GENERAL);
	render_pass.init();
	Vk::window.init_framebuffers(render_pass);

	VkImageView views[] = { offscreen_image.view, depth_image.view };
	VkFramebufferCreateInfo create_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	create_info.attachmentCount = 2;
	create_info.pAttachments = views;
	create_info.width = Vk::window.extent.width;
	create_info.height = Vk::window.extent.height;
	create_info.renderPass = render_pass.ptr;;
	create_info.layers = 1;
	
	if (vkCreateFramebuffer(Vk::device.ptr, &create_info, nullptr, &offscreen_buffer) != VK_SUCCESS) throw std::runtime_error("failed to create framebuffer");

	//BUFFERS

	staging_buffer.init(
		sizeof(uint32_t) * Vk::window.extent.width * Vk::window.extent.height,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	staging_buffer.map();
	std::memset(staging_buffer.data, 0, staging_buffer.size);

	ubo_buffer.init(
		static_cast<uint32_t>(sizeof(UBO)),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
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

	//RAYTRACING

	init_raytracing();
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
		camera_raytracing.udpate_raytracing(camera.pos);

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
			render_cpu();
		else 
			render_raytracing();

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
	startTime = currentTime;
}

void Core::update_ubo(void)
{
	UBO ubo{};
	ubo.model = glm::mat4(1.f);// glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//ubo.model = glm::rotate(ubo.model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
	//ubo.model = glm::rotate(ubo.model, dt * glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));


	ubo.view = camera.view;;;
	//ubo.view = glm::lookAt(glm::vec3(0.0f, -5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.proj = glm::perspective(glm::radians(45.0f), Vk::window.extent.width / (float)Vk::window.extent.height, 0.1f, 100.0f);

	ubo.proj[1][1] *= -1;


	ubo_buffer.memcpy(&ubo, sizeof(ubo));

	ubo.model = camera_raytracing.model;

	camera_raytracing.ubo.memcpy(&ubo, sizeof(ubo));

	rt_camera.viewProj = ubo.view * ubo.proj;
	rt_camera.projInverse = glm::inverse(ubo.proj);
	rt_camera.viewInverse = glm::inverse(ubo.view);

	rt_camera_buffer.memcpy(&rt_camera, sizeof(camera_info_s));

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

void Core::start_render(void)
{
	//command_buffer.begin(true);
	//{
	//	

	//	vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
	//		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
	//	);

	//	vulkan::Image::cpy(command_buffer, Vk::window.extent, offscreen_image.image, Vk::window.images[Vk::window.image_index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//	vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
	//		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	//		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
	//	);
	//}
	//command_buffer.end();
}

void Core::render_raytracing(void)
{
	command_buffer.begin(true);
	{
		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);
		;
		if (stop == false) {
			vkCmdBindPipeline(command_buffer.ptr, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline.ptr);
			vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_pipeline.layout, 0, 1, &rt_pool.get_set(0, 0), 0, nullptr);
			vulkan::RayTracingPipeline::vkCmdTraceRaysKHR(command_buffer.ptr, &sbt.gen_region, &sbt.miss_region, &sbt.hit_region, &sbt.call_region, Vk::window.extent.width, Vk::window.extent.height, 1);
			render_pass.begin(command_buffer, Vk::window.extent, offscreen_buffer);
			render_imgui();
			render_pass.end(command_buffer);
		}


		vulkan::Image::cpy(command_buffer, Vk::window.extent, offscreen_image.image, Vk::window.images[Vk::window.image_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


		vulkan::Image::barrier(command_buffer, Vk::window.images[Vk::window.image_index],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
		);
	}
	command_buffer.end();
	rt_camera.frames.x++;
}

void Core::render_imgui(void)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();


	if (ImGui::BeginMainMenuBar()) {


		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open file")) file_to_load = ask_file("").c_str();
			ImGui::EndMenu();
		}

		//ImGui::ShowDemoWindow();

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

	static int selected = 0;

	if (scene.meshes.empty() == false) {
		int num_rays = rt_camera.num_rays.x;
		int max_bounce = rt_camera.max_bounce.x;
		glm::vec4 color = rt_camera.color;
		int changed = 0;

		if (ImGui::Begin("information window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
			changed += ImGui::InputInt("num rays", (int *)&num_rays);
			changed += ImGui::InputInt("max bounce", (int *)&max_bounce);
			changed += ImGui::InputFloat("light intensity", &light_intensity, 0.01f, 1.0f, "%f");
			changed += ImGui::InputFloat3("test color", (float*)&rt_camera.color);
			changed += ImGui::InputInt("infinity", &infinity);

			rt_camera.num_rays.x = num_rays;
			rt_camera.max_bounce.x = max_bounce;
			rt_camera.frames.y = light_intensity;

			changed += ImGui::InputFloat3("Diffuse", (float*)&materials[selected].diffuse);
			changed += ImGui::InputFloat3("Specular", (float*)&materials[selected].specular);
			changed += ImGui::InputFloat3("Emissive", (float*)&materials[selected].emissive);
			changed += ImGui::SliderFloat("Smooth", &materials[selected].smoooth.x, 0.f, 1.f);
			changed += ImGui::SliderFloat("Specular Probability", &materials[selected].smoooth.y, 0.f, 1.f);
			ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			for (int i = 0; i < scene.materials.size(); i++)
			{
				// FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
				char label[128];
				sprintf_s(label, "Material: %s", scene.materials[i].name.c_str());
				if (ImGui::Selectable(label, selected == i)) {
					selected = i;
					changed++;
				}
			}
			ImGui::EndChild();

			ImGui::End();
		}
		if (changed) {
			materials_buffer.memcpy(materials.data(), sizeof(material_info_s) * materials.size());
			changed = 0;
			rt_camera.frames.x = 0;
		}

	}

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.ptr);
}
void Core::render_gpu(void)
{
	static std::string file_to_load = "";

	if (file_to_load != "") {
		scene.load_scene(file_to_load.c_str());
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
		gizmo.draw(command_buffer);;;

		//vkCmdBindDescriptorSets(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, gizmo.pipeline.layout, 0, 1, &descriptors.get_set(0, 1), 0, nullptr);
		//camera_raytracing.draw(command_buffer);

		render_imgui();

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
	static std::thread l([&] {
		while (true) cpu_raytracing();
	});

	//cpu_raytracing();

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
			core.stop = !core.stop;
			//core.rendering_mode = !core.rendering_mode;
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
	if (ImGui::GetIO().WantCaptureMouse) {
		// ImGui is handling the mouse input, so skip your application's mouse handling logic.
		return;
	}
	if (core.clicked) {
		core.camera.process_mouse(core.dt, xpos - old_xpos, ypos - old_ypos);
		core.rt_camera.frames.x = 0;
	}
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
	core.rt_camera.frames.x = 0;
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

class Sphere {
public:
	Sphere(glm::vec3 _pos, float _radius) : pos(_pos), radius(_radius)
	{

	}
	~Sphere(void) {};

	bool intersectRaySphere(const Ray& ray, hit_info& hitInfo) {
		glm::vec3 oc = ray.orig - pos;
		float a = glm::dot(ray.dir, ray.dir);
		float b = 2.0f * glm::dot(oc, ray.dir);
		float c = glm::dot(oc, oc) - radius * radius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0.0f) {
			return false; // No intersection
		}

		float sqrtDiscriminant = glm::sqrt(discriminant);

		// Find the nearest t value for the intersection
		float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
		float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

		float t = (t1 > 0) ? t1 : t2;
		if (t < 0) {
			return false; // Intersection is behind the ray origin
		}

		// Compute intersection point and normal
		hitInfo.distance = t;
		hitInfo.pos = ray.orig + t * ray.dir;
		hitInfo.normal = glm::normalize(hitInfo.pos - pos);

		return true;
	}

	glm::vec3 pos;
	float radius;

private:
};

#include <random>

static float randomFloat() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(gen);
}

glm::vec3 Core::trace_ray(Ray r, int max_bounce)
{
	glm::vec3 light{};
	glm::vec3 final_color{1.f, 1.f, 1.f};
	bool nothing = true;
	int direct_light = 0;
	//static Sphere sphere({ 0.f, 0.f, 0.f }, 0.5f);

	for (int i = 0; i <= max_bounce; i++) {
		hit_info info;
		hit_info tmp;
		bool found = false;

		for (auto& mesh : scene.meshes) {
			if (!mesh.hit(r, tmp)) continue;
			found = true;
			nothing = false;

			if (tmp.distance < info.distance)
				info = tmp;
		}

		//if (sphere.intersectRaySphere(r, tmp)) {
		//	found = true;
		//	nothing = false;
		//	tmp.material_index = 0;
		//	if (tmp.distance < info.distance)
		//		info = tmp;
		//}

		if (!found) break;

		bool is_specular_bounce = scene.materials[info.material_index].specular_probability > randomFloat();

		light += final_color * scene.materials[info.material_index].emissive;
		final_color *= (is_specular_bounce ? glm::vec3{1.f, 1.f, 1.f} : scene.materials[info.material_index].diffuse);

		//final_color = info.normal;

		glm::vec3 diffuse_direction = Camera::generateRandomDirection(info.normal);
		glm::vec3 specular_direction =  r.dir - 2.0f * glm::dot(r.dir, info.normal) * info.normal;
		
		r.orig = info.pos - r.dir * 0.0001f;;
		r.dir = glm::mix(diffuse_direction, specular_direction, scene.materials[info.material_index].smooth * is_specular_bounce);
		//r.dir = Camera::generateRandomDirection(info.normal);
	}

	if (nothing) return {};

	//return final_color;

	return light;
}

void Core::cpu_raytracing(void)
{
	static bool done = false;

	int width = (int)Vk::window.extent.width;
	int height = (int)Vk::window.extent.height;

	pixel_s* screen = (pixel_s *)staging_buffer.data;

	auto start_point = camera_raytracing.top_left + (camera_raytracing.du + camera_raytracing.dv) / 2.f;
	static int num_frames = 0;

	if (infinity == false && done) return;
	BEGIN_LOOP(width, height);

		Ray new_ray(camera_raytracing.pos, start_point + camera_raytracing.du * (float)x + camera_raytracing.dv * (float)y);
		//screen[index] = {};
		glm::vec3 light = {};

		for (int i = 0; i < rt_camera.num_rays.x ; i++)
			light += trace_ray(new_ray, rt_camera.max_bounce.x);

		glm::vec3 color{ light.x , light.y, light.z};
		color /= (float)rt_camera.num_rays.y;
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

	done = true;
}

void Core::init_raytracing(void)
{
	//UNIFORMS INFO
	rt_camera_buffer.init(
		static_cast<uint32_t>(sizeof(camera_info_s)),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);
	rt_camera_buffer.map();
	rt_camera.max_bounce = { 1.f, 0.f, 0.f, 0.f };;
	rt_camera.num_rays = { 1.f, 0.f, 0.f, 0.f };
	rt_camera.frames = { 0.f, 0.f, 0.f, 0.f };
	rt_camera.color = { 1.f, 0.f, 0.f, 1.f };

	instances_info.init(
		static_cast<uint32_t>(sizeof(instance_info_s) * scene.meshes.size()),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);
	instances_info.map();

	materials_buffer.init(
		static_cast<uint32_t>(sizeof(material_info_s) * scene.materials.size()),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);
	materials_buffer.map();


	std::vector<instance_info_s> addresses;
	for (auto& i : scene.meshes)
		addresses.emplace_back( i.buffer.address, (uint64_t)i.material_index );

	instances_info.memcpy(addresses.data(), sizeof(instance_info_s) * addresses.size());

	for (auto& i : scene.materials) {
		materials.push_back({ {i.diffuse.x, i.diffuse.y, i.diffuse.z, 0}, {}, {} });
		if (i.name == "light_softboxes")
			materials.rbegin()->emissive = {0.85, .43f, .13f, 0.f};
	}
	//materials[5].emissive = { 1.f, 1.f, 1.f, 1.f };
	//scene.materials[5].emissive = materials[5].emissive;
	materials_buffer.memcpy(materials.data(), sizeof(material_info_s) * materials.size());

	//CREATING ACCELERATION SCTRUCTURES

	blases.resize(scene.meshes.size());
	for (int i = 0; i < scene.meshes.size(); i++) {
		blases[i].init({ &scene.meshes[i].buffer });
		blases[i].build(command_buffer);
	}

	tlas.init(blases);
	tlas.build(command_buffer);

	//CREATING DESCRIPTOR SET
	rt_pool.add_set(1)
		.add_binding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) // tlas
		.add_binding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR) // offscreen image
		.add_binding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) // camera
		.add_binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) // array of instance info
		.add_binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR); // array of material info
	rt_pool.init();

	rt_pool.add_write(0, 0, 0, tlas.ptr);
	rt_pool.add_write(0, 0, 1, offscreen_image.view);
	rt_pool.add_write(0, 0, 2, rt_camera_buffer.ptr);
	rt_pool.add_write(0, 0, 3, instances_info.ptr);
	rt_pool.add_write(0, 0, 4, materials_buffer.ptr);
	rt_pool.write();

	for (auto& layout : rt_pool.layouts)
		rt_pipeline.add_layout(layout);

	//CREATING RAYTRACING PIPELINE

	vulkan::Shader gen("shaders/raygen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	vulkan::Shader miss("shaders/raymiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
	vulkan::Shader hit("shaders/raychit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	rt_pipeline.add_shader_stage(gen.stage);
	rt_pipeline.add_shader_stage(miss.stage);
	rt_pipeline.add_shader_stage(hit.stage);

	rt_pipeline.add_shader_group(vulkan::ShaderGroup().type_general().generalShader(0));
	rt_pipeline.add_shader_group(vulkan::ShaderGroup().type_general().generalShader(1));
	rt_pipeline.add_shader_group(vulkan::ShaderGroup().type_triangle().closestHitShader(2));
	
	rt_pipeline.init();

	//SHADER BINDING TABLE

	sbt.init(rt_pipeline.ptr, 1, 1);

	command_buffer.begin(true);
	offscreen_image.barrier(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	command_buffer.end();
	vulkan::Fence f;
	f.init(false);
	command_buffer.submit_p(nullptr, nullptr, &f);
	f.wait();
}