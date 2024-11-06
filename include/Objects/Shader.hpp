#pragma once
#include "Golmon.hpp"

namespace vulkan {

	class Shader {
	public:
		Shader(char const* file_name, VkShaderStageFlagBits stage_flag);
		~Shader(void);

		VkPipelineShaderStageCreateInfo stage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	private:
	};
}