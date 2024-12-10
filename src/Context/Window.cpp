#include <assert.h>
#include <algorithm>
#include "Context/Window.hpp"
#include "Objects/Image.hpp"
#include "Objects/GraphicsPipeline.hpp"
#include "Objects/Sync.hpp"

using namespace vulkan;

Window::Window(void)
{

}

Window::~Window(void)
{
	if (created == false) return;

    for (int i = 0; i < views.size(); i++) {
        vkDestroyImageView(Context::device.ptr, views[i], nullptr);
        if (framebuffers.empty() == false)
            vkDestroyFramebuffer(Context::device.ptr, framebuffers[i], nullptr);
    }
    vkDestroySwapchainKHR(Context::device.ptr, swapchain, nullptr);
	vkDestroySurfaceKHR(Context::instance.ptr, surface, nullptr);
	glfwDestroyWindow(ptr);
}

void Window::init(int w, int h, char const* title)
{
	vulkan::Context::window.created = true;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!(vulkan::Context::window.ptr = glfwCreateWindow(w, h, title, nullptr, nullptr))) throw std::runtime_error("failed to create window");
}

void Window::init_surface(void)
{
	if (glfwCreateWindowSurface(Context::instance.ptr, ptr, nullptr, &surface) != VK_SUCCESS) throw std::runtime_error("failed to create surface");
}

std::vector<char const *> Window::get_extensions(void)
{
	uint32_t count = 0;
	char const** glfw_extensinos = glfwGetRequiredInstanceExtensions(&count);
	return std::vector<char const*>(glfw_extensinos, glfw_extensinos + count);	
}

void Window::init_swapchain(void) {
    VkSwapchainCreateInfoKHR create_info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context::device.physical_ptr, surface, &surface_capabilities) != VK_SUCCESS) throw std::runtime_error("failed to get surface capabilites");

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Context::device.physical_ptr, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Context::device.physical_ptr, surface, &formatCount, formats.data());
    //format = formats[0];
    format.format = VK_FORMAT_B8G8R8A8_UNORM; //sizeof(uint32_t) per pixel
    format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    int w, h;
    glfwGetFramebufferSize(ptr, &w, &h);
    extent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
    extent.width = std::clamp(extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

    create_info.minImageCount = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && create_info.minImageCount > surface_capabilities.maxImageCount)
        create_info.minImageCount = surface_capabilities.maxImageCount;
    create_info.surface = surface;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    create_info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(Context::device.ptr, &create_info, nullptr, &swapchain) != VK_SUCCESS) throw std::runtime_error("failed to create swapchain");

    create_resources();
}

void Window::create_resources(void)
{
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(Context::device.ptr, swapchain, &count, nullptr);
    images.resize(count);
    views.resize(count);
    vkGetSwapchainImagesKHR(Context::device.ptr, swapchain, &count, images.data());

    for (uint32_t i = 0; i < count; i++) {
        VkImageViewCreateInfo create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

        create_info.image = images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = format.format;
        create_info.components = {};
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.layerCount = 1;
        create_info.subresourceRange.levelCount = 1;

        if (vkCreateImageView(Context::device.ptr, &create_info, nullptr, &views[i]) != VK_SUCCESS) throw std::runtime_error("failed to create image views");
    }
}

void Window::init_framebuffers(RenderPass& render_pass)
{
    framebuffers.resize(images.size());

    for (int i = 0; i < images.size(); i++) {
        VkImageView attachments[] = { views[i], nullptr};
        if (render_pass.depth_image)
            attachments[1] = render_pass.depth_image->view;
        VkFramebufferCreateInfo create_info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        create_info.renderPass = render_pass.ptr;
        create_info.attachmentCount = 1 + (render_pass.depth_image != nullptr);
        create_info.pAttachments = attachments;
        create_info.width = extent.width;
        create_info.height = extent.height;
        create_info.layers = 1;

        if (vkCreateFramebuffer(Context::device.ptr, &create_info, nullptr, &framebuffers[i]) != VK_SUCCESS) throw std::runtime_error("failed to create framebuffer");
    }
}

void Window::acquire_next_image(VkSemaphore s, VkFence f)
{
    if (vkAcquireNextImageKHR(Context::device.ptr, swapchain, UINT64_MAX, s, f, &image_index) != VK_SUCCESS) throw std::runtime_error("failed to acquire next image");
}

void Window::present(Semaphore& s)
{
    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &s.ptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = &image_index;

    if (vkQueuePresentKHR(Context::device.queue.present, &present_info) != VK_SUCCESS) throw std::runtime_error("failed to present");
}