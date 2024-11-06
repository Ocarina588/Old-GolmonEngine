#include "Model.hpp"
#include "objloader.hpp"

Mesh::Mesh(void)
{

}

Mesh::~Mesh(void)
{

}

void Mesh::init(void* data, size_t size)
{
	buffer.init(
		data, static_cast<uint32_t>(size),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

void Mesh::init(std::vector<Vertex>& _vertices)
{
	vertices = _vertices;
	init(vertices.data(), vertices.size() * sizeof(Vertex));
}

Model::Model(void)
{

}

Model::~Model(void)
{

}

Scene::Scene(void)
{

}

Scene::~Scene(void)
{

}

void Scene::load_obj(char const* obj)
{
	meshes.clear();
	buffers.clear();
	offsets.clear();

	std::cout << meshes.size() << std::endl;

	objl::Loader loader;
	static int lol = 0;
	printf("%s\n", obj);
	if (lol++)
		if (loader.LoadFile(obj) == false) throw std::runtime_error("failed to load obj");
	else
		if (loader.LoadFile(obj) == false) throw std::runtime_error("failed to load obj");


	meshes.resize(loader.LoadedMeshes.size());

	for (int i = 0; i < loader.LoadedMeshes.size(); i ++) {
		auto& mesh = loader.LoadedMeshes[i];

		std::vector<Vertex> vertices;
		std::vector<glm::vec3> ns(mesh.Indices.size());
		for (int j = 0; j < mesh.Indices.size(); j += 3) {
			glm::vec3* a = (glm::vec3*)&mesh.Vertices[mesh.Indices[j]].Position;
			glm::vec3* b = (glm::vec3*)&mesh.Vertices[mesh.Indices[j + 1]].Position;
			glm::vec3* c = (glm::vec3*)&mesh.Vertices[mesh.Indices[j + 2]].Position;
			glm::vec3 n = glm::normalize(glm::cross(*b - *a, *c - *a));
			ns[mesh.Indices[j]] += n;
			ns[mesh.Indices[j + 1]] += n;
			ns[mesh.Indices[j + 2]] += n;
			vertices.push_back({ {a->x, a->y, a->z},{} });
			vertices.push_back({ {b->x, b->y, b->z},{} });
			vertices.push_back({ {c->x, c->y, c->z},{} });
		}
		for (auto& i : ns)
			i = glm::normalize(i);
		for (int i = 0; i < mesh.Indices.size(); i++)
			vertices[i].color = ns[mesh.Indices[i]];

		meshes[i].init(vertices);
	}

	offsets.resize(meshes.size());
	for (int i = 0; i < meshes.size(); i++) {
		buffers.push_back(meshes[i].buffer.ptr);
		total_size += (uint32_t)meshes[i].vertices.size();
	}

	std::cout << meshes.size() << " " << buffers.size() << " " << total_size << std::endl;
}

void Scene::draw(vulkan::CommandBuffer& b)
{
	for (int i = 0; i < meshes.size(); i++) {
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(b.ptr, 0, 1, &meshes[i].buffer.ptr, &offset);
		vkCmdDraw(b.ptr, meshes[i].buffer.size, 1, 0, 0);
	}
}