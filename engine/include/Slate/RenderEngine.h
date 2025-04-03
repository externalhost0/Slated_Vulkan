//
// Created by Hayden Rivas on 3/17/25.
//

#pragma once
#include <array>

#include "volk.h"
#include "VkBootstrap.h"
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>


#include "Slate/Components.h"
#include "Slate/VK/vkdescriptor.h"
#include "Slate/VK/vktypes.h"
#include "Slate/Resources/ShaderResource.h"
#include "Slate/ShaderPass.h"

namespace Slate {
	struct BootstrapSolution {
		vkb::Instance VkbInstance;
		vkb::PhysicalDevice VkbPhysicalDevice;
		vkb::Device VkbDevice;
	};
	constexpr unsigned int FRAME_OVERLAP = 2;
	// every frame has one of these, because we are double buffer we will assign two
	struct FrameData {
		// sync objects
		VkSemaphore _swapchainSemaphore = VK_NULL_HANDLE, _renderSemaphore = VK_NULL_HANDLE;
		VkFence _renderFence = VK_NULL_HANDLE;
		// commands
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
		// frame descriptors
		DescriptorAllocatorGrowable _frameDescriptors = {};
	};

	class RenderEngine {
	public:
		RenderEngine() = default;
		explicit RenderEngine(VulkanInstanceInfo info) : info(info) {};
		~RenderEngine() = default;

		void Startup();
		void Shutdown();
		void AquireSwapchainFrame();
		void PresentSwapchainFrame();

		// get exact vulkan data
		VkInstance GetInstance() const { return this->_vkInstance; }
		VkDevice GetDevice() const { return this->_vkDevice; }
		VkPhysicalDevice GetPhysDevice() const { return this->_vkPhysicalDevice; }
		VkQueue GetGraphicsQueue() const { return this->_vkGraphicsQueue; }
		VmaAllocator GetAllocator() const { return this->_allocator; }

		// get abstracted data
		VulkanInstanceInfo GetInstanceInfo() const { return this->info; }
		VkExtent2D GetSwapchainExtent() const { return this->_vkSwapchianExtent; }
		uint32_t GetCurrentFrameNum() const { return this->_frameNum; }
		FrameData& GetCurrentFrameData() { return this->_frames[this->_frameNum % FRAME_OVERLAP]; };

		VkImage GetSwapchainImage() const { return this->_vkSwapchainImages[_imageIndex]; }
		VkImageView GetSwapchainImageView() const { return this->_vkSwapchainImageViews[_imageIndex]; }

		void CreateDefaultSamplers();
		void OnResize(uint16_t width, uint16_t height);
		void InjectWindow(GLFWwindow* window) { this->glfwWindow = window; }
		GLFWwindow* glfwWindow = nullptr;

		void DestroySwapchain();
		bool resizeRequested = false;
	public:
		vktypes::AllocatedImage CreateImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples = VK_SAMPLE_COUNT_1_BIT, bool mipmapped = false) const;
		vktypes::AllocatedImage CreateImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples = VK_SAMPLE_COUNT_1_BIT, bool mipmapped = false) const;
		vktypes::AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		vktypes::AllocatedImage CreateImage(const std::filesystem::path& file_path, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

		vktypes::AllocatedBuffer CreateUniformBuffer(size_t size);
		vktypes::AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0) const;
		MeshBuffer CreateMeshBuffer(std::vector<Vertex> vertices);
		MeshBuffer CreateMeshBuffer(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

		void DestroyBuffer(vktypes::AllocatedBuffer& allocatedBuffer) const;
		void DestroyImage(vktypes::AllocatedImage& allocatedImage) const;

		void Immediate_Submit(std::function<void(VkCommandBuffer cmd)>&& function);

		ShaderPass CreateShaderPass(const ShaderResource& resource, PassProperties& properties) const;
	public:
		// helper functions that do small things, might be replaced due to it being macgyvered
		void DrawMeshData_EXT(const MeshBuffer& data);
		VkClearColorValue clearColorValue = { 0.1f, 0.1f, 0.1f, 1.0f };
	public:
		VkSampler default_LinearSampler  = VK_NULL_HANDLE;
		VkSampler default_NearestSampler = VK_NULL_HANDLE;


		// all created images are stored on the Render Engine
		FastVector<vktypes::AllocatedImage, 64> allocatedImages;

	private:
		void CreateInstance();
		void CreateSurface(GLFWwindow* pGlfwWindow);
		void CreateDevices();
		void CreateSwapchain(uint16_t width = 0, uint16_t height = 0);
		void CreateAllocator();
		void CreateQueues();
		void CreateCommands();
		void CreateSyncStructures();
		void CreateFrameDescriptors();
	private:
		// SETUP, single instances
		VkInstance _vkInstance                     = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT _vkDebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR _vkSurfaceKHR                 = VK_NULL_HANDLE;
		VkPhysicalDevice _vkPhysicalDevice         = VK_NULL_HANDLE;
		VkDevice _vkDevice                         = VK_NULL_HANDLE; // This device cant be..
		VkPhysicalDeviceProperties _vkPhysDeviceProperties = {};
		VmaAllocator _allocator                    = VK_NULL_HANDLE; // for global allocations with vma, 1 per device
		VkSwapchainKHR _vkSwapchain                = VK_NULL_HANDLE;

		// our two queues, one for rendering, other for presentaiton on screen
		VkQueue _vkPresentQueue  = VK_NULL_HANDLE; // not used as of now
		VkQueue _vkGraphicsQueue = VK_NULL_HANDLE;
		uint32_t _vkGraphicsQueueFamily = 0;
		uint32_t _imageIndex = 0;

		// swapchain values
		std::vector<VkImage> _vkSwapchainImages = {};
		std::vector<VkImageView> _vkSwapchainImageViews = {};
		VkFormat _swapchainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D _vkSwapchianExtent = {};

		// random ahh info
		VulkanInstanceInfo info;
		BootstrapSolution _bootstrap = {};

		// frame data
		FrameData _frames[FRAME_OVERLAP] = {};
		uint32_t _frameNum = 0;

		// immediate submit structures
		VkFence _immFence                 = VK_NULL_HANDLE;
		VkCommandBuffer _immCommandBuffer = VK_NULL_HANDLE;
		VkCommandPool _immCommandPool     = VK_NULL_HANDLE;
	};
}