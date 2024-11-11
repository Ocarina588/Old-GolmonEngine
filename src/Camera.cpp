#include "Camera.hpp"
#include "Model.hpp"

using Vk = vulkan::Context;

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

#include <random>
#include <glm/gtx/orthonormalize.hpp>

// Helper function to generate a random float between 0 and 1
static float randomFloat() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(gen);
}

// Function to create an orthonormal basis from a given normal vector
static void createOrthonormalBasis(const glm::vec3& normal, glm::vec3& tangent, glm::vec3& bitangent) {
	tangent = glm::abs(normal.x) > 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

	tangent = glm::normalize(glm::cross(normal, tangent));
	bitangent = glm::cross(normal, tangent);
}

// Function to generate a random direction in the hemisphere defined by the normal
static glm::vec3 generateRandomDirection(const glm::vec3& normal) {
	glm::vec3 tangent, bitangent;
	createOrthonormalBasis(normal, tangent, bitangent);

	// Generate random angles for spherical coordinates
	float phi = 2.0f * glm::pi<float>() * randomFloat();
	float cosTheta = randomFloat();  // Cosine-weighted hemisphere
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	// Calculate direction in local space
	glm::vec3 directionLocal = glm::vec3(
		cos(phi) * sinTheta,
		sin(phi) * sinTheta,
		cosTheta
	);

	// Transform to world space
	glm::vec3 direction = directionLocal.x * tangent +
		directionLocal.y * bitangent +
		directionLocal.z * normal;

	return glm::normalize(direction);
}

std::vector<Ray> trace_ray(Ray r, int max_bounce, Scene &scene)
{
	std::vector<Ray> rays;
	std::cout << "?" << std::endl;
	for (int i = 0; i <= max_bounce; i++) {
		hit_info info;
		bool found = false;


		for (auto& mesh : scene.meshes) {
			hit_info tmp;
			std::cout << "allo" << std::endl;
			if (!mesh.hit(r, tmp)) continue;
			found = true;

			if (tmp.distance < info.distance)
				info = tmp;
		}

		std::cout << "lets see" << std::endl;
		if (!found) break;

		std::cout << "new" << std::endl;

		r.orig = info.pos;
		r.dir = generateRandomDirection(info.normal);
		rays.push_back(r);
	}

	return rays;
}

Camera::Camera(glm::vec3 _pos, glm::vec3 _target) : pos(_pos), target(_target)
{

}

Camera::~Camera(void)
{

}

void Camera::update(float dt)
{
	direction = glm::normalize(target - pos);
	right = glm::normalize(glm::cross(world_up, direction));

	view = glm::lookAt(pos, target, world_up);

	model = glm::mat4(1.f);
}

void Camera::udpate_raytracing(glm::vec3 new_pos)
{
	pos = new_pos;
	// DIRECTION
	direction = glm::normalize(target - pos) * focal_length;

	// RIGHT
	right = -glm::normalize(glm::cross(world_up, direction)) * viewport_width * 0.5f;

	// UP
	up = -glm::normalize(glm::cross(direction, right)) * viewport_height * 0.5f;

	// CAMERA FRAME
	top_left = direction + up - right;
	top_right = direction + up + right;
	bottom_left = direction - right - up;
	botton_right = direction + right - up;

	// DU & DV
	du = right * 2.f / (float)Vk::window.extent.width;
	dv = -up * 2.f / (float)Vk::window.extent.height;
}

void print_vector(glm::vec3 const& v)
{
	std::cout << v.x << " " << v.y << " " << v.z;
}

void Camera::init(int w, int h, Scene *scene)
{
	ubo.init(
		static_cast<uint32_t>(sizeof(UBO)),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	ubo.map();

	aspect_ratio = (float)w / (float)h;
	focal_length = 2.f;
	viewport_height = 2.f;
	viewport_width = viewport_height * aspect_ratio;

	update(0.f);
	udpate_raytracing(pos);

	// COLORS 
	glm::vec3 white = { 1.f, 1.f, 1.f };
	glm::vec3 red = { 1.f, 0.f, 0.f };
	glm::vec3 green = { 0.f, 1.f, 0.f }; 
	glm::vec3 blue = { 0.f, 0.f, 1.f };

	std::vector<Vertex> vertices = {

		{pos, white},
		{pos + top_left, white},
		{pos + top_right, white},

		{pos, white},
		{pos + top_left, white},
		{pos + bottom_left, white},

		{pos, white},
		{pos + bottom_left, white},
		{pos + botton_right, white},

		{pos, white},
		{pos + botton_right, white},
		{pos + top_right, white},

		{pos, white},
		{pos + direction, white},
		{pos + direction, white},

	};

	int WIDTH = 10, HEIGHT = 5;
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			auto DU = right * 2.f / (float)WIDTH;
			auto DV = -up * 2.f / (float)HEIGHT;

			auto start_point = top_left + (DU + DV) / 2.f;
			//Ray new_ray(pos, start_point + DU * (float)j + DV * (float)i);
			Ray new_ray(pos, direction);
			
			vertices.push_back({ new_ray.orig, red });
			vertices.push_back({ new_ray.dir, red });
			vertices.push_back({ new_ray.dir, red });

			for (int u = 0; u < 10; u++) {

				auto rays = trace_ray(new_ray, 0, *scene);

				for (auto& tmp : rays) {
					std::cout << "adding" << std::endl;
					vertices.push_back({ tmp.orig, white });
					vertices.push_back({ tmp.dir, white });
					vertices.push_back({ tmp.dir, white });
				}
			}

			break;
		}
		break;
	}

	vertex_buffer.init(
		vertices.data(), static_cast<uint32_t>(sizeof(Vertex) * vertices.size()),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

glm::vec3 rotateAroundPoint(const glm::vec3& point, const glm::vec3& center, glm::vec3 axis, float angle) {
	glm::vec3 translatedPoint = point - center;
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);
	glm::vec4 rotatedPoint = rotationMatrix * glm::vec4(translatedPoint, 1.0f);
	
	return glm::vec3(rotatedPoint) + center;
}

void Camera::process_mouse(float dt, double x, double y)
{
	float speed = 0.0002f;
	pos = rotateAroundPoint(pos, target, { 0.f, 1.f, 0.f }, -(float)x * speed * dt * glm::radians(90.f));
	pos = rotateAroundPoint(pos, target, right, (float)y * speed * dt * glm::radians(90.f));
}

void Camera::process_scroll(float dt, double x, double y)
{

}

void Camera::draw(vulkan::CommandBuffer& b)
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(b.ptr, 0, 1, &vertex_buffer.ptr, &offset);
	vkCmdDraw(b.ptr, vertex_buffer.size, 1, 0, 0);
}