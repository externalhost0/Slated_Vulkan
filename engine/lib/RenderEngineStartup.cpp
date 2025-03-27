//
// Created by Hayden Rivas on 3/17/25.
//
#include "Slate/Debug.h"
#include "Slate/RenderEngine.h"

#include <volk.h>
#include <VkBootstrap.h>
namespace Slate {
	void RenderEngine::Startup() {
		this->CreateInstance();
		this->CreateSurface(this->glfwWindow);
		this->CreateDevices();
		this->CreateSwapchain();
		this->CreateAllocator();
		this->CreateQueues();
		this->CreateCommands();
		this->CreateSyncStructures();
		this->CreateFrameDescriptors();
	}

	void RenderEngine::Shutdown() {
		// wait before destruction process
		vkDeviceWaitIdle(this->_vkDevice);
		{
			// destroy default resources
			{
				vkDestroySampler(this->_vkDevice, default_NearestSampler, nullptr);
				vkDestroySampler(this->_vkDevice, default_LinearSampler, nullptr);
			}
			// destroy imgui resources
			{
//				vkDestroyDescriptorPool(this->_vkDevice, _imguiDescriptorPool, nullptr);
			}
			// destroy vma images
			{
				for (vktypes::AllocatedImage& image : this->allocatedImages) {
					this->DestroyImage(image);
				}
			}
			// destroy imm
			{
				vkDestroyFence(this->_vkDevice, _immFence, nullptr);
				vkFreeCommandBuffers(this->_vkDevice, _immCommandPool, 1, &_immCommandBuffer);
				vkDestroyCommandPool(this->_vkDevice, _immCommandPool, nullptr);
			}
			// destroy framedata (commands and sync structures)
			for (FrameData& _frame: this->_frames) {
				vkDestroyFence(this->_vkDevice, _frame._renderFence, nullptr);
				vkDestroySemaphore(this->_vkDevice, _frame._swapchainSemaphore, nullptr);
				vkDestroySemaphore(this->_vkDevice, _frame._renderSemaphore, nullptr);

				vkFreeCommandBuffers(this->_vkDevice, _frame._commandPool, 1, &_frame._commandBuffer);
				vkDestroyCommandPool(this->_vkDevice, _frame._commandPool, nullptr);

				_frame._frameDescriptors.DestroyPools(this->_vkDevice); // per frame descriptor pool destruction
			}
			// destroy swapchain & its resources
			// because we already have this functionality implemented for runtime swapchain recreation we just reuse it here
			this->DestroySwapchain();
			// destroy single objects
			vmaDestroyAllocator(this->_allocator);// if you get validation errors regarding "device memory not destroyed" its this guy
			vkDestroySurfaceKHR(this->_vkInstance, this->_vkSurfaceKHR, nullptr);
			vkDestroyDevice(this->_vkDevice, nullptr);
			vkb::destroy_debug_utils_messenger(this->_vkInstance, this->_vkDebugMessenger);
			vkDestroyInstance(this->_vkInstance, nullptr);
		}
	}

	void RenderEngine::CreateInstance() {
		EXPECT(volkInitialize() == VK_SUCCESS, "Volk failed to initialize!"); // kinda important

		vkb::InstanceBuilder builder;
		auto instance_result = builder
									   .set_app_name(this->info.app_name)
									   .set_app_version(this->info.app_version.major, this->info.app_version.minor, this->info.app_version.patch)
									   .set_engine_name(this->info.engine_name)
									   .set_engine_version(this->info.engine_version.major, this->info.engine_version.minor, this->info.engine_version.patch)

									   .request_validation_layers()
									   .use_default_debug_messenger()
									   .require_api_version(VK_API_VERSION_1_3)
									   .build();
		EXPECT(instance_result.has_value(), "Failed to create Vulkan instance. Error: {}", instance_result.error().message().c_str());

		vkb::Instance& vkbinstance = instance_result.value();
		this->_bootstrap.VkbInstance = vkbinstance;
		this->_vkInstance = vkbinstance.instance;
		this->_vkDebugMessenger = vkbinstance.debug_messenger;

		volkLoadInstance(this->_vkInstance);
	}
	void RenderEngine::CreateSurface(GLFWwindow* pGlfwWindow) {
		EXPECT(glfwCreateWindowSurface(this->_vkInstance, pGlfwWindow, nullptr, &this->_vkSurfaceKHR) == VK_SUCCESS, "Failed surface creation");
	}
	void RenderEngine::CreateDevices() {
		// vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.synchronization2 = true;
		features13.dynamicRendering = true;

		//vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.descriptorIndexing = true;
		features12.bufferDeviceAddress = true;

		// vulkan 1.1 features
		VkPhysicalDeviceVulkan11Features features11 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };

		//vulkan 1.0 features
		VkPhysicalDeviceFeatures features = {};
		features.fragmentStoresAndAtomics = true;
		features.fillModeNonSolid = true;
		features.independentBlend = true;
		features.shaderInt64 = true;

		vkb::PhysicalDeviceSelector selector(_bootstrap.VkbInstance);
		auto physical_result = selector
				.set_minimum_version(1, 2)
				.add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
				.add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
				.add_required_extension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)
				.add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
						//  .set_required_features_13(features13) //cant be on until 1.3 mvk
				.set_required_features_12(features12)
				.set_required_features_11(features11)
				.set_required_features(features)
				.set_surface(_vkSurfaceKHR)
				.select();
		EXPECT(physical_result.has_value(), "Failed to select Vulkan Physical Device. Error: {}", physical_result.error().message().c_str())

		vkb::PhysicalDevice& vkbphysdevice = physical_result.value();
		this->_bootstrap.VkbPhysicalDevice = vkbphysdevice;
		this->_vkPhysicalDevice = vkbphysdevice.physical_device;
		this->_vkPhysDeviceProperties = vkbphysdevice.properties;

		// i have no idea why this is a physica device feature but im adding it into the logical device builder
		// ok so basically to not set off the validation layer, we need to use this on the physical device, we can uncomment required features 13 when 1.3 is ready for moltenvk
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		dynamic_rendering_feature.pNext = nullptr;
		dynamic_rendering_feature.dynamicRendering = VK_TRUE;

		VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR };
		synchronization2_feature.pNext = nullptr;
		synchronization2_feature.synchronization2 = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		dynamic_state_feature.pNext = nullptr;
		dynamic_state_feature.extendedDynamicState = VK_TRUE;


		// GET DEVICE
		vkb::DeviceBuilder device_builder{this->_bootstrap.VkbPhysicalDevice};
		auto device_result = device_builder
				.add_pNext(&dynamic_rendering_feature)
				.add_pNext(&synchronization2_feature)
				.add_pNext(&dynamic_state_feature)
				.build();
		EXPECT(device_result.has_value(), "Failed to create Vulkan device. Error: {}", device_result.error().message().c_str());

		vkb::Device& vkbdevice = device_result.value();
		this->_bootstrap.VkbDevice = vkbdevice;
		this->_vkDevice = vkbdevice.device;

		volkLoadDevice(this->_vkDevice);
	}
	void RenderEngine::CreateSwapchain(uint16_t width, uint16_t height) {
		vkb::SwapchainBuilder swapchainBuilder(this->_vkPhysicalDevice, this->_vkDevice, this->_vkSurfaceKHR);
		auto swapchain_result = swapchainBuilder
										.set_desired_format(VkSurfaceFormatKHR{
												.format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM,
												.colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
										.set_desired_present_mode(VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR)// VERY IMPORTANT, decides framerate/buffer/sync
										.set_desired_extent(width, height)
										.add_image_usage_flags(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
										.build();
		EXPECT(swapchain_result.has_value(), "[VULKAN] {}", swapchain_result.error().message().c_str());

		vkb::Swapchain& vkbswapchain = swapchain_result.value();
		this->_vkSwapchain = vkbswapchain.swapchain;
		this->_vkSwapchainImageViews = vkbswapchain.get_image_views().value();
		this->_vkSwapchainImages = vkbswapchain.get_images().value();
		this->_swapchainImageFormat = vkbswapchain.image_format;
		this->_vkSwapchianExtent = vkbswapchain.extent;
	}
	void RenderEngine::CreateAllocator() {
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = this->_vkPhysicalDevice;
		allocatorInfo.device = this->_vkDevice;
		allocatorInfo.instance = this->_vkInstance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		const VmaVulkanFunctions functions = {
				.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
				.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};
		allocatorInfo.pVulkanFunctions = &functions;
		EXPECT(vmaCreateAllocator(&allocatorInfo, &this->_allocator) == VK_SUCCESS, "Failed to create vma Allocator!");
	}
	void RenderEngine::CreateQueues() {
		// graphics queue
		auto graphics_queue_result = this->_bootstrap.VkbDevice.get_queue(vkb::QueueType::graphics);
		EXPECT(graphics_queue_result.has_value(), "Failed to get graphics queue. Error: {}", graphics_queue_result.error().message().c_str())
		this->_vkGraphicsQueue = graphics_queue_result.value();

		// graphics queue index
		auto graphics_queue_index_result = this->_bootstrap.VkbDevice.get_queue_index(vkb::QueueType::graphics);
		EXPECT(graphics_queue_index_result.has_value(), "Failed to get graphics queue index/family. Error: {}", graphics_queue_index_result.error().message().c_str())
		this->_vkGraphicsQueueFamily = graphics_queue_index_result.value();

		// present queue
		auto present_queue_result = this->_bootstrap.VkbDevice.get_queue(vkb::QueueType::present);
		EXPECT(present_queue_result.has_value(), "Failed to get present queue. Error: {}", present_queue_result.error().message().c_str())
		this->_vkPresentQueue = present_queue_result.value();
	}
	void RenderEngine::CreateCommands() {
		// this needs to be altered later depending on the operations my engine needs
		VkCommandPoolCreateInfo poolInfo = vkinfo::CreateCommandPoolInfo(this->_vkGraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		for (FrameData& _frame: this->_frames) {
			VK_CHECK(vkCreateCommandPool(this->_vkDevice, &poolInfo, nullptr, &_frame._commandPool));

			VkCommandBufferAllocateInfo allocInfo = vkinfo::CreateCommandBufferAllocateInfo(_frame._commandPool, 1);
			VK_CHECK(vkAllocateCommandBuffers(this->_vkDevice, &allocInfo, &_frame._commandBuffer));
		}

		// imm
		VK_CHECK(vkCreateCommandPool(this->_vkDevice, &poolInfo, nullptr, &this->_immCommandPool));
		VkCommandBufferAllocateInfo immCBAI = vkinfo::CreateCommandBufferAllocateInfo(this->_immCommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(this->_vkDevice, &immCBAI, &this->_immCommandBuffer));
	}
	void RenderEngine::CreateSyncStructures() {
		VkFenceCreateInfo fenceInfo = vkinfo::CreateFenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		VkSemaphoreCreateInfo semaphoreInfo = vkinfo::CreateSemaphoreInfo();

		for (FrameData& _frame: this->_frames) {
			VK_CHECK(vkCreateFence(this->_vkDevice, &fenceInfo, nullptr, &_frame._renderFence));

			VK_CHECK(vkCreateSemaphore(this->_vkDevice, &semaphoreInfo, nullptr, &_frame._swapchainSemaphore));
			VK_CHECK(vkCreateSemaphore(this->_vkDevice, &semaphoreInfo, nullptr, &_frame._renderSemaphore));
		}
		// imm
		VK_CHECK(vkCreateFence(this->_vkDevice, &fenceInfo, nullptr, &this->_immFence));
	}
	void RenderEngine::CreateFrameDescriptors() {
		for (FrameData& _frame: this->_frames) {
			// create a descriptor pool
			std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
					{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
					{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3},
					{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
			};
			_frame._frameDescriptors = DescriptorAllocatorGrowable {};
			_frame._frameDescriptors.Init(this->_vkDevice, 1000, frame_sizes);
		}
	}

	void RenderEngine::AquireSwapchainFrame() {
		VK_CHECK(vkWaitForFences(this->_vkDevice, 1, &this->GetCurrentFrameData()._renderFence, VK_TRUE, UINT64_MAX));
		this->GetCurrentFrameData()._frameDescriptors.ClearPools(_vkDevice);

		VkResult aquireResult = vkAcquireNextImageKHR(
				this->_vkDevice,
				this->_vkSwapchain,
				UINT64_MAX,
				this->GetCurrentFrameData()._swapchainSemaphore,
				nullptr,
				&this->_imageIndex);
		if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR || aquireResult == VK_SUBOPTIMAL_KHR || resizeRequested) {
			this->resizeRequested = true;
			if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		VK_CHECK(vkResetFences(this->_vkDevice, 1, &this->GetCurrentFrameData()._renderFence));
		VK_CHECK(vkResetCommandPool(this->_vkDevice, this->GetCurrentFrameData()._commandPool, 0));
	}
	void RenderEngine::PresentSwapchainFrame() {
		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(this->GetCurrentFrameData()._commandBuffer);
		VkSemaphoreSubmitInfo waitInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, this->GetCurrentFrameData()._swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, this->GetCurrentFrameData()._renderSemaphore);

		VkSubmitInfo2 submitInfo = vkinfo::CreateSubmitInfo(&cmdinfo, &signalInfo, &waitInfo);
		vkQueueSubmit2KHR(this->_vkGraphicsQueue, 1, &submitInfo, this->GetCurrentFrameData()._renderFence);

		VkPresentInfoKHR presentinfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = nullptr };
		presentinfo.pSwapchains = &this->_vkSwapchain;
		presentinfo.swapchainCount = 1;
		presentinfo.pWaitSemaphores = &this->GetCurrentFrameData()._renderSemaphore;
		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pImageIndices = &this->_imageIndex;

		VkResult presentResult = vkQueuePresentKHR(this->_vkGraphicsQueue, &presentinfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || resizeRequested) {
			this->resizeRequested = true;
			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		this->_frameNum++;
	}
	void RenderEngine::DestroySwapchain() {
		for (VkImageView imageView : this->_vkSwapchainImageViews) {
			vkDestroyImageView(this->_vkDevice, imageView, nullptr);
		}
		// images are destroyed alongside swapchain destruction
		vkDestroySwapchainKHR(this->_vkDevice, this->_vkSwapchain, nullptr);
		this->_vkSwapchainImages.clear();
		this->_vkSwapchainImageViews.clear();
	}
	void RenderEngine::OnResize(uint16_t width, uint16_t height) {
		vkDeviceWaitIdle(this->_vkDevice);
		{
			// create swapchain
			this->DestroySwapchain();
			this->CreateSwapchain(width, height);
		}
		this->resizeRequested = false;
	}

	void RenderEngine::CreateDefaultSamplers() {
		VkSamplerCreateInfo linearInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
		linearInfo.magFilter = VK_FILTER_LINEAR;
		linearInfo.minFilter = VK_FILTER_LINEAR;
		linearInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		linearInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		linearInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		linearInfo.minLod = -1000;
		linearInfo.maxLod = 1000;
		linearInfo.maxAnisotropy = 1.0f;
		vkCreateSampler(this->_vkDevice, &linearInfo, nullptr, &this->default_LinearSampler);

		VkSamplerCreateInfo nearestInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
		nearestInfo.magFilter = VK_FILTER_NEAREST;
		nearestInfo.minFilter = VK_FILTER_NEAREST;
		nearestInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		nearestInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		nearestInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		nearestInfo.minLod = -1000;
		nearestInfo.maxLod = 1000;
		nearestInfo.maxAnisotropy = 1.0f;
		vkCreateSampler(this->_vkDevice, &nearestInfo, nullptr, &this->default_NearestSampler);
	}
}