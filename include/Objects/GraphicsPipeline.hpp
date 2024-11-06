#pragma once
#include "Context/Context.hpp"
#include "Objects/Commands.hpp"

namespace vulkan {
	class Image;
	class CommandBuffer;
	class RenderPass {
	public:
		friend class GraphicsPipeline;
		friend class Window;
		RenderPass(void);
		~RenderPass(void);

		void init(void);

		inline void use_depth(Image& _depth_image) { subpass_description.pDepthStencilAttachment = &depth_ref; depth_image = &_depth_image; }
		inline void set_final_layout(VkImageLayout layout) { color_attachment.finalLayout = layout; }
		inline void set_initial_layout(VkImageLayout layout) { color_attachment.initialLayout = layout; }

		void begin(CommandBuffer& buffer, VkExtent2D extent, VkFramebuffer framebuffer);
		void end(CommandBuffer& buffer);

		VkRenderPass ptr = nullptr;
	private:
		VkAttachmentDescription color_attachment;
		VkAttachmentDescription depth_attachment;
		VkAttachmentReference color_ref{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depth_ref{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass_description;
		Image* depth_image = nullptr;
	};

	class GraphicsPipeline {
	public:
		GraphicsPipeline(void);
		~GraphicsPipeline(void);

		inline void add_shader_stage(VkPipelineShaderStageCreateInfo i) { stages.push_back(i); }
		inline void add_binding(VkVertexInputBindingDescription i) { bindings.push_back(i); }
		inline void add_attribute(VkVertexInputAttributeDescription i) { attributes.push_back(i); }
		inline void add_layout(VkDescriptorSetLayout i) { layouts.push_back(i); }

		inline void set_render_pass(RenderPass &_render_pass) { 
			render_pass = _render_pass.ptr; 
			if (_render_pass.subpass_description.pDepthStencilAttachment == nullptr) return;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		}
		void init(void);

		inline void bind(CommandBuffer& command_buffer) { vkCmdBindPipeline(command_buffer.ptr, VK_PIPELINE_BIND_POINT_GRAPHICS, ptr); }

		VkPipelineLayout layout = nullptr;
		VkPipeline ptr = nullptr;
	private:

		VkRenderPass render_pass = nullptr;
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkDescriptorSetLayout> layouts;
		VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

	};
}