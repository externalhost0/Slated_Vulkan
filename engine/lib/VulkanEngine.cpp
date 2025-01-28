//
// Created by Hayden Rivas on 1/14/25.
//
// external headers
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

// my headers
#include "Slate/Debug.h"
#include "Slate/Expect.h"
#include "Slate/VulkanEngine.h"
#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkext.h"
#include "Slate/Components.h"

namespace Slate {
	void VulkanEngine::CreateInstance(const VulkanInstanceInfo &info) {
		vkb::InstanceBuilder builder;
		auto instance_result = builder
									   .set_app_name(info.app_name)
									   .set_app_version(info.app_version.major, info.app_version.minor, info.app_version.patch)
									   .set_engine_name(info.engine_name)
									   .set_engine_version(info.engine_version.major, info.engine_version.minor, info.engine_version.patch)

									   .request_validation_layers()
									   .use_default_debug_messenger()
									   .require_api_version(VK_API_VERSION_1_3)
									   .build();
		if (!instance_result) {
			EXPECT(false, "Failed to create Vulkan instance. Error: {}", instance_result.error().message().c_str())
		}
		auto &vkbinstance = instance_result.value();

		_bootstrap.VkbInstance = vkbinstance;
		_vkInstance = vkbinstance.instance;
		_vkDebugMessenger = vkbinstance.debug_messenger;
	}
	void VulkanEngine::CreateSurface(GLFWwindow *pGlfwWindow) {
		// surface creation
		if (glfwCreateWindowSurface(_vkInstance, pGlfwWindow, nullptr, &_vkSurfaceKHR) != VK_SUCCESS) {
			EXPECT(false, "Vulkan & GLFW surface creation failed!")
		}
	}
	void VulkanEngine::CreateDevices() {
		// GET PHYSICAL DEVICE
		//vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features = {};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features.synchronization2 = true;
		features.dynamicRendering = true;

		//vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12 = {};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.descriptorIndexing = true;
		features12.bufferDeviceAddress = true;

		//vulkan 1.0 features
		VkPhysicalDeviceFeatures features10 = {};
		features10.fillModeNonSolid = true;

		vkb::PhysicalDeviceSelector selector(_bootstrap.VkbInstance);
		auto physical_result = selector
									   .set_minimum_version(1, 2)
									   .add_required_extension("VK_KHR_dynamic_rendering")
									   .add_required_extension("VK_KHR_synchronization2")
									   .add_required_extension("VK_KHR_copy_commands2")
									   //									   .set_required_features_13(features) //cant be on until 1.3 mvk
									   .set_required_features_12(features12)
									   .set_required_features(features10)
									   .set_surface(_vkSurfaceKHR)
									   .select();
		if (!physical_result) {
			EXPECT(false, "Failed to select Vulkan Physical Device. Error: {}", physical_result.error().message().c_str())
		}
		auto &vkbphysdevice = physical_result.value();

		_bootstrap.VkbPhysicalDevice = vkbphysdevice;
		_vkPhysicalDevice = vkbphysdevice.physical_device;


		// i have no idea why this is a physica device feature but im adding it into the logical device builder
		// ok so basically to not set off the validation layer, we need to use this on the physical device, we can uncomment required features 13 when 1.3 is ready for moltenvk
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {};
		dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamic_rendering_feature.pNext = nullptr;
		dynamic_rendering_feature.dynamicRendering = VK_TRUE;

		VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_feature = {};
		synchronization2_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
		synchronization2_feature.pNext = nullptr;
		synchronization2_feature.synchronization2 = VK_TRUE;

		// GET DEVICE
		vkb::DeviceBuilder device_builder{_bootstrap.VkbPhysicalDevice};
		auto device_result = device_builder
									 .add_pNext(&dynamic_rendering_feature)
									 .add_pNext(&synchronization2_feature)
									 .build();
		if (!device_result) {
			EXPECT(false, "Failed to create Vulkan device. Error: {}", device_result.error().message().c_str())
		}
		auto &vkbdevice = device_result.value();

		_bootstrap.VkbDevice = vkbdevice;
		_vkDevice = vkbdevice.device;
	}
	void VulkanEngine::CreateAllocator() {
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = _vkPhysicalDevice;
		allocatorInfo.device = _vkDevice;
		allocatorInfo.instance = _vkInstance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &_allocator);
	}

	void VulkanEngine::CreateQueues() {
		// GETTING QUEUES!

		// graphics queue
		auto graphics_queue_result = _bootstrap.VkbDevice.get_queue(vkb::QueueType::graphics);
		if (!graphics_queue_result) {
			EXPECT(false, "Failed to get graphics queue. Error: {}", graphics_queue_result.error().message().c_str())
		}
		_vkGraphicsQueue = graphics_queue_result.value();
		// graphics queue index
		auto graphics_queue_index_result = _bootstrap.VkbDevice.get_queue_index(vkb::QueueType::graphics);
		if (!graphics_queue_index_result) {
			EXPECT(false, "Failed to get graphics queue index/family. Error: {}", graphics_queue_index_result.error().message().c_str())
		}
		_vkGraphicsQueueFamily = graphics_queue_index_result.value();

		// present queue
		auto present_queue_result = _bootstrap.VkbDevice.get_queue(vkb::QueueType::present);
		if (!present_queue_result) {
			EXPECT(false, "Failed to get present queue. Error: {}", present_queue_result.error().message().c_str())
		}
		_vkPresentQueue = present_queue_result.value();
	}

	void VulkanEngine::BuildSwapchain(uint16_t width, uint16_t height) {
		vkb::SwapchainBuilder swapchainBuilder(_vkPhysicalDevice, _vkDevice, _vkSurfaceKHR);
		auto swapchain_result = swapchainBuilder
										.set_desired_format(VkSurfaceFormatKHR{
												.format = VkFormat::VK_FORMAT_B8G8R8A8_UNORM,
												.colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
										.set_desired_present_mode(VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR)// VERY IMPORTANT, decides framerate/buffer/sync
										.set_desired_extent(width, height)
										.add_image_usage_flags(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
										.build();
		if (!swapchain_result) {
			EXPECT(false, "[VULKAN] {}", swapchain_result.error().message().c_str())
		}
		vkb::Swapchain &vkbswapchain = swapchain_result.value();
		_vkSwapchain = vkbswapchain.swapchain;
		_vkSwapchainImageViews = vkbswapchain.get_image_views().value();
		_vkSwapchainImages = vkbswapchain.get_images().value();
		_swapchainImageFormat = vkbswapchain.image_format;
		_vkSwapchianExtent = vkbswapchain.extent;
	}


	void VulkanEngine::CreateSwapchain() {
		// Create Swapchain is only for initialization, should not be run again!!
		BuildSwapchain();

		VkExtent3D tempDrawImageExtent = {
				_vkSwapchianExtent.width,
				_vkSwapchianExtent.height,
				1};
		// COLOR
		{
			_colorImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
			_colorImage.imageExtent = tempDrawImageExtent;

			VkImageUsageFlags drawImageUsages = {};
			drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
			drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

			VkImageCreateInfo rimg_info = vkinfo::CreateImageInfo(_colorImage.imageFormat, drawImageUsages, tempDrawImageExtent);

			//for the draw image, we want to allocate it from gpu local memory
			VmaAllocationCreateInfo rimg_allocinfo = {};
			rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			//allocate and create the image
			vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_colorImage.image, &_colorImage.allocation, nullptr);

			//build a image-view for the draw image to use for rendering
			VkImageViewCreateInfo rview_info = vkinfo::CreateImageViewInfo(_colorImage.imageFormat, _colorImage.image, VK_IMAGE_ASPECT_COLOR_BIT);
			VK_CHECK(vkCreateImageView(_vkDevice, &rview_info, nullptr, &_colorImage.imageView));
		}
		// DEPTH
		{
			_depthStencilImage.imageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
			_depthStencilImage.imageExtent = tempDrawImageExtent;
			VkImageUsageFlags depthImageUsages = {};
			depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			VkImageCreateInfo dimg_info = vkinfo::CreateImageInfo(_depthStencilImage.imageFormat, depthImageUsages, tempDrawImageExtent);

			// same as above drawimage alllocinfo
			VmaAllocationCreateInfo rimg_allocinfo = {};
			rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			//allocate and create the image
			vmaCreateImage(_allocator, &dimg_info, &rimg_allocinfo, &_depthStencilImage.image, &_depthStencilImage.allocation, nullptr);

			//build a image-view for the draw image to use for rendering
			VkImageViewCreateInfo dview_info = vkinfo::CreateImageViewInfo(_depthStencilImage.imageFormat, _depthStencilImage.image, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			VK_CHECK(vkCreateImageView(_vkDevice, &dview_info, nullptr, &_depthStencilImage.imageView));
		}
	}

	void VulkanEngine::RebuildSwapchain(uint16_t width, uint16_t height) {
		// CLEAN
		vkDeviceWaitIdle(_vkDevice);
		DestroySwapchain();
		// RECREATE
		BuildSwapchain(width, height);
	}
	void VulkanEngine::DestroySwapchain() {
		for (VkImageView imageView: _vkSwapchainImageViews) {
			vkDestroyImageView(_vkDevice, imageView, nullptr);
		}
		vkDestroySwapchainKHR(_vkDevice, _vkSwapchain, nullptr);
	}

	void VulkanEngine::OnResizeSwapchain(GLFWwindow *pWindow) {
		{
			int w, h;
			glfwGetWindowSize(pWindow, &w, &h);
			RebuildSwapchain(w, h);
		}
		resizeRequested = false;
	}

	void VulkanEngine::CreateCommands() {
		// this needs to be altered later depending on the operations my engine needs
		VkCommandPoolCreateInfo poolInfo = vkinfo::CreateCommandPoolInfo(_vkGraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);


		for (FrameData &_frame: _frames) {
			VK_CHECK(vkCreateCommandPool(_vkDevice, &poolInfo, nullptr, &_frame._commandPool));

			VkCommandBufferAllocateInfo allocInfo = vkinfo::CreateCommandBufferAllocateInfo(_frame._commandPool, 1);
			VK_CHECK(vkAllocateCommandBuffers(_vkDevice, &allocInfo, &_frame._commandBuffer));
		}

		// imm
		VK_CHECK(vkCreateCommandPool(_vkDevice, &poolInfo, nullptr, &_immCommandPool));
		VkCommandBufferAllocateInfo immCBAI = vkinfo::CreateCommandBufferAllocateInfo(_immCommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(_vkDevice, &immCBAI, &_immCommandBuffer));
	}

	void VulkanEngine::CreateSyncStructures() {
		VkFenceCreateInfo fenceInfo = vkinfo::CreateFenceInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		VkSemaphoreCreateInfo semaphoreInfo = vkinfo::CreateSemaphoreInfo();

		for (FrameData &_frame: _frames) {
			VK_CHECK(vkCreateFence(_vkDevice, &fenceInfo, nullptr, &_frame._renderFence));

			VK_CHECK(vkCreateSemaphore(_vkDevice, &semaphoreInfo, nullptr, &_frame._swapchainSemaphore));
			VK_CHECK(vkCreateSemaphore(_vkDevice, &semaphoreInfo, nullptr, &_frame._renderSemaphore));
		}

		// imm
		VK_CHECK(vkCreateFence(_vkDevice, &fenceInfo, nullptr, &_immFence));
	}

	void VulkanEngine::Destroy() {
		// wait before destruction process
		vkDeviceWaitIdle(_vkDevice);
		{
			for (auto buffer: this->allIndexBuffers) {
				buffer.Destroy(_allocator);
			}

			/// destroy our pipelines
			{
				vkDestroyPipelineLayout(_vkDevice, _vkMainPipelineLayout, nullptr);
				vkDestroyPipeline(_vkDevice, _standardPipeline, nullptr);
			}
			// destroy imgui resources
			{
				vkDestroyDescriptorPool(_vkDevice, imguiDescriptorPool, nullptr);
			}
			// destroy vma images
			{
				_colorImage.Destroy(_vkDevice, _allocator);
				_depthStencilImage.Destroy(_vkDevice, _allocator);
			}

			// destroy imm
			vkDestroyFence(_vkDevice, _immFence, nullptr);
			vkFreeCommandBuffers(_vkDevice, _immCommandPool, 1, &_immCommandBuffer);
			vkDestroyCommandPool(_vkDevice, _immCommandPool, nullptr);

			// destroy framedata (commands and sync structures)
			for (FrameData &_frame: _frames) {
				vkDestroyFence(_vkDevice, _frame._renderFence, nullptr);
				vkDestroySemaphore(_vkDevice, _frame._swapchainSemaphore, nullptr);
				vkDestroySemaphore(_vkDevice, _frame._renderSemaphore, nullptr);

				vkFreeCommandBuffers(_vkDevice, _frame._commandPool, 1, &_frame._commandBuffer);
				vkDestroyCommandPool(_vkDevice, _frame._commandPool, nullptr);
			}

			// destroy swapchain & its resources
			// because we already have this functionality implemented for runtime swapchain recreation we just reuse it here
			{
				DestroySwapchain();
			}
			// destroy single objects
			{
				vmaDestroyAllocator(_allocator);// if you get validation errors regarding "device memory not destroyed" its this guy
				vkDestroySurfaceKHR(_vkInstance, _vkSurfaceKHR, nullptr);
				vkDestroyDevice(_vkDevice, nullptr);
				vkb::destroy_debug_utils_messenger(_vkInstance, _vkDebugMessenger);
				vkDestroyInstance(_vkInstance, nullptr);
			}
		}
	}

	void VulkanEngine::Immediate_Submit(std::function<void(VkCommandBuffer cmd)> &&function) {
		VK_CHECK(vkResetFences(_vkDevice, 1, &_immFence));
		VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

		VkCommandBufferBeginInfo cmdBeginInfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VK_CHECK(vkBeginCommandBuffer(_immCommandBuffer, &cmdBeginInfo));

		function(_immCommandBuffer);

		VK_CHECK(vkEndCommandBuffer(_immCommandBuffer));

		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(_immCommandBuffer);
		VkSubmitInfo2 submit = vkinfo::CreateSubmitInfo(&cmdinfo, nullptr, nullptr);

		// submit command buffer to the queue and execute it.
		//  _renderFence will now block until the graphic commands finish execution
		VK_CHECK(vkext::vkQueueSubmit2(_vkGraphicsQueue, 1, &submit, _immFence));

		VK_CHECK(vkWaitForFences(_vkDevice, 1, &_immFence, true, 9999999999));
	}

	vktypes::AllocatedBuffer VulkanEngine::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) const {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = memoryUsage;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;


		vktypes::AllocatedBuffer newBuffer = {};
		VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));
		return newBuffer;
	}

	void VulkanEngine::Draw(vktypes::MeshData data, VkPipelineLayout layout) {
		vktypes::GPUDrawPushConstants pushConstants = {};
		pushConstants.vertexBufferAddress = data.constants.vertexBufferAddress;
		pushConstants.renderMatrix = data.constants.renderMatrix;
		vkCmdPushConstants(this->getCurrentFrameData()._commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vktypes::GPUDrawPushConstants), &pushConstants);

		if (data.IsIndexed()) {
			vkCmdBindIndexBuffer(this->getCurrentFrameData()._commandBuffer, data.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(this->getCurrentFrameData()._commandBuffer, data.indexCount, 1, 0, 0, 0);
		} else {
			vkCmdDraw(this->getCurrentFrameData()._commandBuffer, data.vertexCount, 1, 0, 0);
		}
	}

	vktypes::MeshData VulkanEngine::CreateMesh(std::vector<Vertex> vertices) {
		// just calculate sizes of our parameters
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);


		vktypes::MeshData mesh = {};
		mesh.vertexCount = vertices.size();
		vktypes::GPUMeshBuffers surfaceBuffer = {};

		// create vertex buffer
		surfaceBuffer.vertexBuffer = CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		//find the adress of the vertex buffer
		VkBufferDeviceAddressInfo deviceAdressInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr};
		deviceAdressInfo.buffer = surfaceBuffer.vertexBuffer.buffer;
		mesh.constants.vertexBufferAddress = vkGetBufferDeviceAddress(_vkDevice, &deviceAdressInfo);


		vktypes::AllocatedBuffer staging = CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *data = staging.allocation->GetMappedData();

		// copy vertex buffer
		memcpy(data, vertices.data(), vertexBufferSize);

		Immediate_Submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertexCopy{0};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, surfaceBuffer.vertexBuffer.buffer, 1, &vertexCopy);
		});

		staging.Destroy(_allocator);
		return mesh;
	}

	vktypes::MeshData VulkanEngine::CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
		// just calculate sizes of our parameters
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		vktypes::MeshData mesh = {};
		mesh.indexCount = indices.size();
		vktypes::GPUMeshBuffers newSurface = {};

		// create vertex buffer
		newSurface.vertexBuffer = CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		//find the adress of the vertex buffer
		VkBufferDeviceAddressInfo deviceAdressInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr};
		deviceAdressInfo.buffer = newSurface.vertexBuffer.buffer;
		mesh.constants.vertexBufferAddress = vkGetBufferDeviceAddress(_vkDevice, &deviceAdressInfo);

		//create index buffer
		mesh.indexBuffer = CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		vktypes::AllocatedBuffer staging = CreateBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *data = staging.allocation->GetMappedData();

		// copy vertex buffer
		memcpy(data, vertices.data(), vertexBufferSize);
		// copy index buffer
		memcpy((char *) data + vertexBufferSize, indices.data(), indexBufferSize);

		Immediate_Submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertexCopy{0};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

			VkBufferCopy indexCopy{0};
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, mesh.indexBuffer.buffer, 1, &indexCopy);
		});

		newSurface.Destroy(_allocator);
		staging.Destroy(_allocator);
		return mesh;
	}

	void VulkanEngine::Aquire() {
		VK_CHECK(vkWaitForFences(this->_vkDevice, 1, &this->getCurrentFrameData()._renderFence, VK_TRUE, UINT64_MAX));
		VK_CHECK(vkResetFences(this->_vkDevice, 1, &this->getCurrentFrameData()._renderFence));

		VkResult aquireResult = vkAcquireNextImageKHR(
				this->_vkDevice,
				this->_vkSwapchain,
				UINT64_MAX,
				this->getCurrentFrameData()._swapchainSemaphore,
				nullptr,
				&this->_imageIndex);
		if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR || aquireResult == VK_SUBOPTIMAL_KHR) {
			this->resizeRequested = true;
			if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		VK_CHECK(vkResetCommandPool(this->_vkDevice, this->getCurrentFrameData()._commandPool, 0));
	}

	void VulkanEngine::Present() {
		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(this->getCurrentFrameData()._commandBuffer);
		VkSemaphoreSubmitInfo waitInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, this->getCurrentFrameData()._swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, this->getCurrentFrameData()._renderSemaphore);
		VkSubmitInfo2 submitInfo = vkinfo::CreateSubmitInfo(&cmdinfo, &signalInfo, &waitInfo);
		vkext::vkQueueSubmit2(this->_vkGraphicsQueue, 1, &submitInfo, this->getCurrentFrameData()._renderFence);


		VkPresentInfoKHR presentinfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = nullptr };
		presentinfo.pSwapchains = &this->_vkSwapchain;
		presentinfo.swapchainCount = 1;
		presentinfo.pWaitSemaphores = &this->getCurrentFrameData()._renderSemaphore;
		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pImageIndices = &this->_imageIndex;

		VkResult presentResult = vkQueuePresentKHR(this->_vkGraphicsQueue, &presentinfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
			this->resizeRequested = true;
			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		this->_frameNum++;
	}


}