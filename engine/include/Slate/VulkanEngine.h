//
// Created by Hayden Rivas on 1/14/25.
//

#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include "Slate/VK/vktypes.h"

namespace Slate {

	constexpr unsigned int FRAME_OVERLAP = 2;
	// every frame has one of these, because we are double buffer we will assign two
	struct FrameData {
		// sync objects
		VkSemaphore _swapchainSemaphore, _renderSemaphore;
		VkFence _renderFence;
		// commands
		VkCommandPool _commandPool;
		VkCommandBuffer _commandBuffer;
	};

	// required for setting up other parts of the Vulkan initialization
	struct BootstrapSolution {
		vkb::Instance VkbInstance;
		vkb::PhysicalDevice VkbPhysicalDevice;
		vkb::Device VkbDevice;
	};

	class VulkanEngine {
	public:
		VulkanEngine() = default;
		~VulkanEngine() = default;
	public:
		// creation functions
		// must be done in this order
		void CreateInstance(const VulkanInstanceInfo & info);
		void CreateSurface(GLFWwindow* pWindow);
		void CreateDevices();
		void CreateAllocator();
		void CreateQueues();
		void CreateSwapchain();
		void CreateCommands();
		void CreateSyncStructures();
	public:
		// runtime functions
		void BuildSwapchain(uint16_t width = 0, uint16_t height = 0);
		void RebuildSwapchain(uint16_t width = 0, uint16_t height = 0);
		void ResizeSwapchain(GLFWwindow *pWindow);
		bool resizeRequested = false;

	public:
		// for meshes
		vktypes::AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) const;
		void DestroyBuffer(const vktypes::AllocatedBuffer& buffer) const;
		void Immediate_Submit(std::function<void(VkCommandBuffer cmd)>&& function);

		vktypes::GPUMeshBuffers UploadMesh(std::vector<Vertex> vertices);
		vktypes::GPUMeshBuffers UploadMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		vktypes::GPUMeshBuffers UploadMesh(std::span<Vertex> vertices, std::span<uint32_t> indices);
	public:
		void Destroy();
	private:
		// only needed upon recreateswapchain() and final destroy()
		void DestroySwapchain();
	public:
		// needed for some initializing logic when using vkBootstrap
		BootstrapSolution _bootstrap = {};
	public:
		// SETUP, single instances
		VkInstance _vkInstance                     = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT _vkDebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR _vkSurfaceKHR                 = VK_NULL_HANDLE;
		VkPhysicalDevice _vkPhysicalDevice         = VK_NULL_HANDLE;
		VkDevice _vkDevice                         = VK_NULL_HANDLE; // This device cant be..
		VmaAllocator _allocator                    = VK_NULL_HANDLE; // for global allocations, 1 per device
		VkSwapchainKHR _vkSwapchain                = VK_NULL_HANDLE;
	public:
		// our two queues, one for rendering, other for presentaiton on screen
		VkQueue _vkPresentQueue = VK_NULL_HANDLE; // not used as of now
		VkQueue _vkGraphicsQueue = VK_NULL_HANDLE;
		uint32_t _vkGraphicsQueueFamily = static_cast<uint32_t>(-1);
	public:
		// swapchain values
		std::vector<VkImage> _vkSwapchainImages;
		std::vector<VkImageView> _vkSwapchainImageViews;
		VkFormat _swapchainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D _vkSwapchianExtent = {};

		// draw resources
		uint32_t _imageIndex = static_cast<uint32_t>(-1);
		vktypes::AllocatedImage _drawImage = {};
		vktypes::AllocatedImage _depthImage = {};
	private:
		FrameData _frames[FRAME_OVERLAP] = {};
	public:
		uint32_t _frameNum = 0;
		FrameData& getCurrentFrameData() { return _frames[_frameNum % FRAME_OVERLAP]; };
	public:
		// currently used by imgui:
		VkDescriptorPool imguiDescriptorPool   = VK_NULL_HANDLE;
	public:
		// our main pipeline
		VkPipeline _vkMainPipeline             = VK_NULL_HANDLE;
		VkPipelineLayout _vkMainPipelineLayout = VK_NULL_HANDLE;
	public:
		// immediate submit structures
		VkFence _immFence                 = VK_NULL_HANDLE;
		VkCommandBuffer _immCommandBuffer = VK_NULL_HANDLE;
		VkCommandPool _immCommandPool     = VK_NULL_HANDLE;
	public:
		glm::vec3 transformtest{};
	};

}