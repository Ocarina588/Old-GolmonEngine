#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "utils.hpp"
#include "Context/Context.hpp"
#include "Objects/Buffer.hpp"
#include "Objects/Commands.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

struct UBO {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Mesh {
public:
	Mesh(void);
	~Mesh(void);

	void init(void* data, size_t size);
	void init(std::vector<Vertex>& _vertices);

	vulkan::Buffer buffer;
	std::vector<Vertex> vertices;

private:
};

class Model {
public:
	Model(void);
	~Model(void);
private:
};

class Scene {
public:
	Scene(void);
	~Scene(void);

	void draw(vulkan::CommandBuffer& b);

	void load_obj(char const* file_name);
	std::vector<Mesh> meshes;
	std::vector<VkBuffer> buffers;
	std::vector<VkDeviceSize> offsets;
	uint32_t total_size = 0;
private:
};