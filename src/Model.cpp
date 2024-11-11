#define DEFINE_OBJLOADER
#include "Model.hpp"

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

static bool intersectTriangle(
	Ray& ray,
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

void Mesh::init(std::vector<Vertex>& _vertices)
{
	vertices = _vertices;
	init(vertices.data(), vertices.size() * sizeof(Vertex));
}

bool Mesh::hit(Ray& r, hit_info &info)
{
	bool found = false;
	float distance = 0.f;
	for (int i = 0; i < vertices.size(); i += 3) {
		auto const& a = vertices[i];
		auto const& b = vertices[i + 1];
		auto const& c = vertices[i + 2];
		glm::vec3 tmp{};

		if (intersectTriangle(r, a.pos, b.pos, c.pos, distance, tmp)) {
			if (distance < info.distance) {
				info.distance = distance;
				info.normal = a.color;
				info.pos = tmp;
				info.material = material;
				found = true;
			}
		}
	}
	return found;
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
		for (int j = 0; j < mesh.Indices.size(); j++)
			vertices[j].color = ns[mesh.Indices[j]];

		meshes[i].init(vertices);
		meshes[i].material = mesh.MeshMaterial;
		std::cout << mesh.MeshName << " " << mesh.MeshMaterial.illum << std::endl;
		meshes[i].name = mesh.MeshName;
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