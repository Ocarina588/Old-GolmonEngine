#define DEFINE_OBJLOADER
#include "Model.hpp"
#include <map>

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
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
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
				info.normal = a.normal;
				info.pos = tmp;
				info.material_index = material_index;
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
	//meshes.clear();
	//buffers.clear();
	//offsets.clear();

	//std::cout << meshes.size() << std::endl;

	//objl::Loader loader;
	//static int lol = 0;
	//printf("%s\n", obj);
	//if (lol++)
	//	if (loader.LoadFile(obj) == false) throw std::runtime_error("failed to load obj");
	//else
	//	if (loader.LoadFile(obj) == false) throw std::runtime_error("failed to load obj");


	//meshes.resize(loader.LoadedMeshes.size());

	//for (int i = 0; i < loader.LoadedMeshes.size(); i ++) {
	//	auto& mesh = loader.LoadedMeshes[i];
	//	
	//	std::vector<Vertex> vertices;
	//	std::vector<glm::vec3> ns(mesh.Indices.size());
	//	for (int j = 0; j < mesh.Indices.size(); j += 3) {
	//		glm::vec3* a = (glm::vec3*)&mesh.Vertices[mesh.Indices[j]].Position;
	//		glm::vec3* b = (glm::vec3*)&mesh.Vertices[mesh.Indices[j + 1]].Position;
	//		glm::vec3* c = (glm::vec3*)&mesh.Vertices[mesh.Indices[j + 2]].Position;
	//		glm::vec3 n = glm::normalize(glm::cross(*b - *a, *c - *a));
	//		ns[mesh.Indices[j]] += n;
	//		ns[mesh.Indices[j + 1]] += n;
	//		ns[mesh.Indices[j + 2]] += n;
	//		vertices.push_back({ {a->x, a->y, a->z},{} });
	//		vertices.push_back({ {b->x, b->y, b->z},{} });
	//		vertices.push_back({ {c->x, c->y, c->z},{} });
	//	}
	//	for (auto& i : ns)
	//		i = glm::normalize(i);
	//	for (int j = 0; j < mesh.Indices.size(); j++)
	//		vertices[j].normal = ns[mesh.Indices[j]];

	//	meshes[i].init(vertices);
	//	meshes[i].material = mesh.MeshMaterial;
	//	std::cout << mesh.MeshName << " " << mesh.MeshMaterial.illum << std::endl;
	//	meshes[i].name = mesh.MeshName;
	//}

	//offsets.resize(meshes.size());
	//for (int i = 0; i < meshes.size(); i++) {
	//	buffers.push_back(meshes[i].buffer.ptr);
	//	total_size += (uint32_t)meshes[i].vertices.size();
	//}

	//std::cout << meshes.size() << " " << buffers.size() << " " << total_size << std::endl;
}

void Scene::load_scene(std::string const& file_name, vulkan::CommandBuffer &co)
{
	meshes.clear();
	buffers.clear();
	offsets.clear();

	
	Assimp::Importer importer;
	//importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS | aiComponent_TEXCOORDS);
	std::cout << "going to load" << std::endl;
	auto flags =
		aiProcess_SortByPType /*aiProcess_RemoveComponent*/ | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals | aiProcess_FlipUVs;
	aiScene const * scene = importer.ReadFile(file_name,  flags);

	if (!scene)
		throw std::runtime_error(importer.GetErrorString());

	std::cout << "Loading Scene:" << std::endl;
	std::cout << "Meshes: " << scene->mNumMeshes << std::endl;
	std::cout << "Textures: " << scene->mNumTextures << std::endl;
	meshes.resize(scene->mNumMeshes);
	buffers.resize(scene->mNumMeshes);
	materials.resize(scene->mNumMaterials);
	textures.resize(scene->mNumTextures);

	for (int i = 0; i < scene->mNumTextures; i++) {
		auto texture = scene->mTextures[i];
		if (texture->mHeight) continue;
		std::cout << "texture " << i << ": " << texture->mWidth << " " << texture->mHeight << std::endl;
		textures[i].init_compressed(
			co,
			texture->pcData, texture->mWidth * 4,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
	}

	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		std::cout << "loading mesh " << i << " over " << scene->mNumMeshes << " faces: " << mesh->mNumFaces << std::endl;
		std::cout << mesh->mNumVertices << std::endl;
		for (int j = 0; j < mesh->mNumFaces; j++) {
			for (int k = 0; k < 3; k++) {
				int index = mesh->mFaces[j].mIndices[k];
				glm::vec3 text_coord = {};
				
				if (mesh->HasTextureCoords(0))
					text_coord = {(float)mesh->mTextureCoords[0][index].x, (float)mesh->mTextureCoords[0][index].y, (float)mesh->mTextureCoords[0][index].z};

				meshes[i].vertices.push_back({
						.pos = {(float)mesh->mVertices[index].x, (float)mesh->mVertices[index].y, (float)mesh->mVertices[index].z},
						//{1.f, 1.f, 0.f}
						.normal = {(float)mesh->mNormals[index].x, (float)mesh->mNormals[index].y, (float)mesh->mNormals[index].z},

						.text_coord = text_coord
						//{(float)mesh->mTextureCoords[j]->x, (float)mesh->mTextureCoords[j]->y}
						//{0.f, 0.f}
					});;
			}
		}



		//for (int j = 0; j < mesh->mNumFaces; j++)
		//	for (int k = 0; k < 3; k++) {
		//		int index = mesh->mFaces[j].mIndices[k];
		//		meshes[i].vertices[j * 3 + k].normal = glm::normalize(m[index]);
		//	}
		//
		meshes[i].init(meshes[i].vertices.data(), sizeof(Vertex) * meshes[i].vertices.size());
		
		
		if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, *(aiColor3D*)&materials[mesh->mMaterialIndex].diffuse) != AI_SUCCESS)
			materials[mesh->mMaterialIndex].diffuse = { 1.f, 0.f, 0.f };
		if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_SPECULAR, *(aiColor3D*)&materials[mesh->mMaterialIndex].specular) != AI_SUCCESS)
			materials[mesh->mMaterialIndex].specular = { 0.f, 0.f, 0.f };
		if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_EMISSIVE_INTENSITY, *(aiColor3D*)&materials[mesh->mMaterialIndex].emissive) != AI_SUCCESS)
			materials[mesh->mMaterialIndex].specular = { 0.f, 0.f, 0.f };

		meshes[i].material_index = mesh->mMaterialIndex;
		meshes[i].texture_index = 0;

		if (scene->mMaterials[mesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString texturePath;
			if (scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
				if (texturePath.data[0] == '*') {
					int textureIndex = atoi(texturePath.C_Str() + 1);
					meshes[i].texture_index = textureIndex + 1;
				}
			}
		}

		materials[mesh->mMaterialIndex].name = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
	}
	//std::cout << "vertices: " << meshes[0].vertices.size() << std::endl;


}

void Scene::draw(vulkan::CommandBuffer& b)
{
	for (int i = 0; i < meshes.size(); i++) {
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(b.ptr, 0, 1, &meshes[i].buffer.ptr, &offset);
		vkCmdDraw(b.ptr, meshes[i].buffer.size, 1, 0, 0);
	}
}