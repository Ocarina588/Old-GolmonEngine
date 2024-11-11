#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Golmon.hpp"

class Camera {
public:
	Camera(glm::vec3 _pos, glm::vec3 look);
	~Camera(void);

	void init(int w = 0, int h = 0, Scene *scene = nullptr);
	void draw(vulkan::CommandBuffer& b);
	void update(float dt);
	void udpate_raytracing(glm::vec3 new_pos);
	void process_mouse(float dt, double x, double y);
	void process_scroll(float dt, double x, double y);

	static glm::vec3 generateRandomDirection(const glm::vec3& normal);

	glm::vec3 pos;
	glm::vec3 target;
	glm::vec3 direction;
	glm::vec3 world_up{ 0.f, 1.f, 0.f };
	glm::vec3 up;
	glm::vec3 right;

	float aspect_ratio;
	float viewport_height;
	float viewport_width;
	float focal_length;

	glm::vec3 top_left;
	glm::vec3 top_right;
	glm::vec3 bottom_left;
	glm::vec3 botton_right;

	glm::vec3 du, dv;

	glm::mat4 view;
	glm::mat4 model;

	vulkan::Buffer ubo, vertex_buffer;
private:
};