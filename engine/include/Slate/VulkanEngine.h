//
// Created by Hayden Rivas on 1/14/25.
//

#pragma once
#include <cstdint>
#include <span>
#include <vector>
#include <filesystem>

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include "VK/vkdescriptor.h"
#include "VK/vktypes.h"

class GLFWwindow;
namespace Slate { struct MeshComponent; }

namespace Slate {
	constexpr unsigned int FRAME_OVERLAP = 2;

	struct EngineStatistics {
		float frametime;
		int triangle_count;
		int drawcall_count;
		float scene_update_count;
		float mesh_draw_time;
	};

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
		VulkanEngine(const VulkanEngine&) = delete;
		VulkanEngine& operator=(const VulkanEngine&) = delete;
	public:
		// creation functions
		// must be done in this order
		void CreateInstance(const VulkanInstanceInfo & info);
		void CreateSurface(GLFWwindow* pWindow);
		void CreateDevices();
		void CreateSwapchain(uint16_t width = 0, uint16_t height = 0);
		void CreateAllocator();

		void CreateQueues();
		void CreateCommands();
		void CreateSyncStructures();
		void CreateStandardSamplers();

		void CreateDescriptors();
		void CreateDefaultImages();
		void CreateDefaultBuffers();
	public:
		void Aquire(); // opening
		void Present(); // closing
		void DestroyEngine();

		// runtime functions
		void OnResizeWindow(GLFWwindow *pWindow);
		bool resizeRequested = false;
	public:
		vktypes::AllocatedImage CreateImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples = VK_SAMPLE_COUNT_1_BIT, bool mipmapped = false) const;
		vktypes::AllocatedImage CreateImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples = VK_SAMPLE_COUNT_1_BIT, bool mipmapped = false) const;

		vktypes::AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0) const;


		vktypes::MeshData CreateMesh(std::vector<Vertex_Standard> vertices);
		vktypes::MeshData CreateMesh(std::vector<Vertex_Standard> vertices, std::vector<uint32_t> indices);
//		vktypes::MeshData CreateMesh(const std::filesystem::path& file_path);

		void Immediate_Submit(std::function<void(VkCommandBuffer cmd)>&& function);
		void DrawMeshData(vktypes::MeshData data, VkPipelineLayout layout);

		void DestroyBuffer(const vktypes::AllocatedBuffer& allocatedBuffer) const;
		void DestroyImage(const vktypes::AllocatedImage& allocatedImage) const;
		void DestroyPipeline(const vktypes::PipelineObject& pipelineObject) const;

		void UpdateDescriptorSets(const vktypes::GPUSceneData& scene_data);
	private:
		void DestroySwapchain();
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
		VkQueue _vkPresentQueue  = VK_NULL_HANDLE; // not used as of now
		VkQueue _vkGraphicsQueue = VK_NULL_HANDLE;
		uint32_t _vkGraphicsQueueFamily = static_cast<uint32_t>(-1);
	public:
		// swapchain values
		std::vector<VkImage> _vkSwapchainImages = {};
		std::vector<VkImageView> _vkSwapchainImageViews = {};
		VkFormat _swapchainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D _vkSwapchianExtent = {};
		// draw resources
		uint32_t _imageIndex = static_cast<uint32_t>(-1);
		// all images
		vktypes::AllocatedImage _drawImage             = {};
		vktypes::AllocatedImage _colorMSAAImage        = {};
		vktypes::AllocatedImage _depthStencilMSAAImage = {};
		vktypes::AllocatedImage _entityImage           = {};
		vktypes::AllocatedImage _entityMSAAImage       = {};
		vktypes::AllocatedImage _viewportImage         = {};
	public:
		uint32_t _frameNum = 0;
		FrameData& getCurrentFrameData() { return _frames[_frameNum % FRAME_OVERLAP]; };
	private:
		FrameData _frames[FRAME_OVERLAP];
	public:
		vktypes::PipelineObject _standardPipeline;
		vktypes::PipelineObject _gridPipeline;
	public:
		// immediate submit structures
		VkFence _immFence                 = VK_NULL_HANDLE;
		VkCommandBuffer _immCommandBuffer = VK_NULL_HANDLE;
		VkCommandPool _immCommandPool     = VK_NULL_HANDLE;
	public:
		VkSampler default_LinearSampler  = VK_NULL_HANDLE;
		VkSampler default_NearestSampler = VK_NULL_HANDLE;
	public:
		VkClearColorValue clearColorValue = { 0.2f, 0.4f, 0.65f, 1.0f };

		// required to move our descriptor data

		VkDescriptorSet _gpuDescriptorSet;
		VkDescriptorSetLayout _gpuDescriptorSetLayout;
		vktypes::AllocatedBuffer _gpuSceneDataBuffer;

		// currently used by imgui:
		VkDescriptorPool imguiDescriptorPool   = VK_NULL_HANDLE;
		VkDescriptorSet _viewportImageDescriptorSet = {};

		// currently not used yet!
		DescriptorAllocatorGrowable _globalDescriptorAllocator;
	};
}