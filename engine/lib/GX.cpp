//
// Created by Hayden Rivas on 4/16/25.
//
#include <Slate/VK/vkimpl.h>

#include "Slate/GX.h"
#include "Slate/Common/Debug.h"
#include "Slate/Common/Logger.h"

#include "Slate/Resources/MeshResource.h"

#include "Slate/VK/vktypes.h"
#include "Slate/VK/vkutil.h"

#include "Slate/CommandBuffer.h"
#include "Slate/VulkanSwapchain.h"


#include <GLFW/glfw3.h>

#include <utility>

namespace Slate {
	const char* toString(VkImageLayout layout);
	// bindings that should match the declarations of injected code into Slang shader,
	// https://github.com/corporateshark/lightweightvk/blob/master/lvk/vulkan/VulkanClasses.cpp#L57
	enum Bindings {
		kGlobalBinding = 0,

		kTextureBinding = 0,
		kSamplerBinding = 1,
		kStorageImageBinding = 2,

		kNumBindlessBindings = 3,
	};

	VkMemoryPropertyFlags StorageTypeToVkMemoryPropertyFlags(StorageType storage) {
		VkMemoryPropertyFlags memFlags{0};
		switch (storage) {
			case StorageType::Device:
				memFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			case StorageType::HostVisible: // dangerous
				memFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				break;
			case StorageType::Memoryless:
				memFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
				break;
		}
		return memFlags;
	}
	VkSemaphore CreateTimelineSemaphore(VkDevice device, unsigned int numImages) {
		const VkSemaphoreTypeCreateInfo smeaphore_type_ci = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
				.initialValue = numImages,
		};
		const VkSemaphoreCreateInfo semaphore_ci = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = &smeaphore_type_ci,
		};
		VkSemaphore semaphore;
		VK_CHECK(vkCreateSemaphore(device, &semaphore_ci, nullptr, &semaphore));
		return semaphore;
	}

	void GX::create(VulkanInstanceInfo info, GLFWwindow* glfWwindow) {
		_backend.initialize(glfWwindow, info);
		// always initialize right after backend
		_imm = CreateUniquePtr<VulkanImmediateCommands>(_backend.getDevice(), _backend.getGraphicsQueueFamilyIndex());
		_staging = CreateUniquePtr<VulkanStagingDevice>(*this);
		// default textures
		{
			// pattern xor
			const uint32_t texWidth = 256;
			const uint32_t texHeight = 256;
			std::vector<uint32_t> pixels(texWidth * texHeight);
			for (uint32_t y = 0; y != texHeight; y++) {
				for (uint32_t x = 0; x != texWidth; x++) {
					pixels[y * texWidth + x] =
							0xFF000000 + ((x^y) << 16) + ((x^y) << 8) + (x^y);
				}
			}
			_dummyTextureHandle = this->createTexture({
					.dimension = {texWidth, texHeight },
					.usage = TextureUsageBits::TextureUsageBits_Sampled | TextureUsageBits::TextureUsageBits_Storage,
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.data = pixels.data(),
					.debugName = "Dummy Texture"
			});
		}
		// ALWAYS initialize swapchain after dummy textures, as swapcahin creates texture handles of its own!!
		_swapchain = CreateUniquePtr<VulkanSwapchain>(*this);
		// timeline semaphore is closely kept to vulkan swapchain
		_timelineSemaphore = CreateTimelineSemaphore(_backend.getDevice(), _swapchain->getNumOfSwapchainImages() - 1);

		// default samplers
		{
			_linearSamplerHandle = this->createSampler({
					.magFilter = SamplerFilter::Linear,
					.minFilter = SamplerFilter::Linear,
					.wrapU = SamplerWrap::Clamp,
					.wrapV = SamplerWrap::Clamp,
					.wrapW = SamplerWrap::Clamp,
					.mipMap = SamplerMip::Disabled,
					.debugName = "Linear Sampler"
			});
			_nearestSamplerHandle = this->createSampler({
					.magFilter = SamplerFilter::Nearest,
					.minFilter = SamplerFilter::Nearest,
					.wrapU = SamplerWrap::Clamp,
					.wrapV = SamplerWrap::Clamp,
					.wrapW = SamplerWrap::Clamp,
					.mipMap = SamplerMip::Disabled,
					.debugName = "Nearest Sampler"
			});
		}

		// GLOBAL DESCRIPTOR SET CREATION
		{
			// default descriptor set
			_globalBufferHandle = this->createBuffer({
					.size = sizeof(GPU::PerFrameData),
					.usage = BufferUsageBits::BufferUsageBits_Uniform,
					.storage = StorageType::Device,
					.debugName = "Global Buffer Data"
			});

			// make global descriptor set layout
			{
				VkDescriptorSetLayoutBinding binding = {
						.binding = 0,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1,
						.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				};
				VkDescriptorSetLayoutCreateInfo dsl_ci = {
						.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
						.bindingCount = 1, // must be at least the same value of the binding itself
						.pBindings = &binding,
				};
				VK_CHECK(vkCreateDescriptorSetLayout(_backend.getDevice(), &dsl_ci, nullptr, &_vkGlobalDSL));
			}
			// make global descriptor set
			{
				VkDescriptorPoolSize poolSize = {
						.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1,
				};
				VkDescriptorPoolCreateInfo poolInfo = {
						.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
						.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
						.maxSets = 1,
						.poolSizeCount = 1,
						.pPoolSizes = &poolSize,
				};
				VK_CHECK(vkCreateDescriptorPool(_backend.getDevice(), &poolInfo, nullptr, &_vkGlobalDPool));
				const VkDescriptorSetAllocateInfo ds_ai = {
						.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
						.descriptorPool = _vkGlobalDPool,
						.descriptorSetCount = 1,
						.pSetLayouts = &_vkGlobalDSL,
				};
				VK_CHECK(vkAllocateDescriptorSets(_backend.getDevice(), &ds_ai, &_vkGlobalDSet));
			}
			// update descriptor set to point to global buffer we created
			{
				const VkDescriptorBufferInfo bufferInfo = {
						.buffer = _bufferPool.get(_globalBufferHandle)->_vkBuffer,
						.offset = 0,
						.range = sizeof(GPU::PerFrameData),
				};
				VkWriteDescriptorSet write = {
						.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						.dstSet = _vkGlobalDSet,
						.dstBinding = kGlobalBinding,
						.dstArrayElement = 0,
						.descriptorCount = 1,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.pBufferInfo = &bufferInfo,
				};
				vkUpdateDescriptorSets(_backend.getDevice(), 1, &write, 0, nullptr);
			}
		}

		growDescriptorPool(_currentMaxTextureCount, _currentMaxSamplerCount);

	}
	void GX::destroy() {
		VK_CHECK(vkDeviceWaitIdle(_backend.getDevice()));
		// more like awaitingDestruction :0
		_awaitingCreation = true;

		_staging.reset(nullptr);
		_swapchain.reset(nullptr);
		vkDestroySemaphore(_backend.getDevice(), _timelineSemaphore, nullptr);
		destroy(_dummyTextureHandle);
		destroy(_globalBufferHandle);
		destroy(_linearSamplerHandle);
		destroy(_nearestSamplerHandle);


		// TODO::fixing the allocation not being proerly freed for VMA
		if (_shaderPool.numObjects()) {
			LOG_USER(LogType::Warning, "Leaked {} shader modules", _shaderPool.numObjects());
			for (int i = 0; i < _shaderPool._objects.size(); i++) {
				destroy(_shaderPool.getHandle(_shaderPool.findObject(&_shaderPool._objects[i]._obj).index()));
			}
		}
		if (_pipelinePool.numObjects()) {
			LOG_USER(LogType::Warning, "Leaked {} render pipelines", _pipelinePool.numObjects());
			for (int i = 0; i < _pipelinePool._objects.size(); i++) {
				destroy(_pipelinePool.getHandle(_pipelinePool.findObject(&_pipelinePool._objects[i]._obj).index()));
			}
		}
		if (_samplerPool.numObjects() > 1) {
			// the dummy value is owned by the context
			LOG_USER(LogType::Warning, "Leaked {} samplers", _samplerPool.numObjects() - 1);
			for (int i = 0; i < _samplerPool._objects.size(); i++) {
				destroy(_samplerPool.getHandle(_samplerPool.findObject(&_samplerPool._objects[i]._obj).index()));
			}
		}
		if (_texturePool.numObjects()) {
			LOG_USER(LogType::Warning, "Leaked {} textures", _texturePool.numObjects());
			for (int i = 0; i < _texturePool._objects.size(); i++) {
				destroy(_texturePool.getHandle(_texturePool.findObject(&_texturePool._objects[i]._obj).index()));
			}
		}
		if (_bufferPool.numObjects()) {
			LOG_USER(LogType::Warning, "Leaked {} buffers", _bufferPool.numObjects());
			for (int i = 0; i < _bufferPool._objects.size(); i++) {
				destroy(_bufferPool.getHandle(_bufferPool.findObject(&_bufferPool._objects[i]._obj).index()));
			}
		}
		// wipe buffers
		_bufferPool.clear();
		_texturePool.clear();
		_samplerPool.clear();
		_shaderPool.clear();
		_pipelinePool.clear();

		waitDeferredTasks();

		_imm.reset(nullptr);

		vkDestroyDescriptorSetLayout(_backend.getDevice(), _vkGlobalDSL, nullptr);
		vkDestroyDescriptorPool(_backend.getDevice(), _vkGlobalDPool, nullptr);
		vkDestroyDescriptorSetLayout(_backend.getDevice(), _vkDSL, nullptr);
		vkDestroyDescriptorPool(_backend.getDevice(), _vkDPool, nullptr);

		_backend.terminate();
	}
	VkDeviceAddress GX::gpuAddress(BufferHandle handle, size_t offset) {
		const AllocatedBuffer* buf = _bufferPool.get(handle);
		ASSERT_MSG(buf && buf->_vkDeviceAddress, "Buffer doesnt have a valid device address!");
		return buf->_vkDeviceAddress + offset;
	}
	CommandBuffer& GX::acquireCommand() {
		ASSERT_MSG(!_currentCommandBuffer._gxCtx, "Cannot acquireSwap more than 1 Command Buffer simultaneously!");

		_currentCommandBuffer = CommandBuffer(this);
		return _currentCommandBuffer;
	}
	void GX::submitCommand(CommandBuffer& cmd, TextureHandle texture) {
		ASSERT(cmd._gxCtx);
		ASSERT(cmd._wrapper);
		bool itspresenttime = !texture.empty();
		if (itspresenttime) {
			const AllocatedImage* tex = _texturePool.get(texture);
			ASSERT_MSG(tex->isSwapchainImage(), "Submitted texture must be swapchain image!");

			const uint64_t signalValue = _swapchain->_currentFrameNum + _swapchain->getNumOfSwapchainImages();
			_swapchain->_timelineWaitValues[_swapchain->_currentImageIndex] = signalValue;
			_imm->signalSemaphore(_timelineSemaphore, signalValue);
		}
		cmd._lastSubmitHandle = _imm->submit(*cmd._wrapper);
		if (itspresenttime) {
			_swapchain->present();
		}
		processDeferredTasks();
		SubmitHandle handle = cmd._lastSubmitHandle;
		// reset
		_currentCommandBuffer = {};
	}
	void GX::resizeSwapchain() {
		vkDeviceWaitIdle(_backend.getDevice());
		int w, h;
		glfwGetFramebufferSize(_windowService.getFocusedWindow()->getGLFWWindow(), &w, &h);
		// delete
		_swapchain.reset(nullptr);
		vkDestroySemaphore(_backend.getDevice(), _timelineSemaphore, nullptr);
		// create
		_swapchain = CreateUniquePtr<VulkanSwapchain>(*this, w, h);
		_timelineSemaphore = CreateTimelineSemaphore(_backend.getDevice(), _swapchain->getNumOfSwapchainImages() - 1);
	}
	BufferHandle GX::createBuffer(BufferSpec spec) {
		 VkBufferUsageFlags usage_flags = (spec.storage == StorageType::Device) ? VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT : 0;

		if (spec.usage & BufferUsageBits::BufferUsageBits_Index)
			usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (spec.usage & BufferUsageBits::BufferUsageBits_Uniform)
			usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (spec.usage & BufferUsageBits::BufferUsageBits_Storage)
			usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (spec.usage & BufferUsageBits::BufferUsageBits_Indirect)
			usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		ASSERT_MSG(usage_flags, "Invalid buffer creation specification!");
		const VkMemoryPropertyFlags mem_flags = StorageTypeToVkMemoryPropertyFlags(spec.storage);

		AllocatedBuffer obj = this->createBufferImpl(spec.size, usage_flags, mem_flags);
		snprintf(obj._debugName, sizeof(obj._debugName) - 1, "%s", spec.debugName);
		BufferHandle handle = _bufferPool.create(std::move(obj));
		if (spec.data) {
			upload(handle, spec.data, spec.size);
		}
		_awaitingCreation = true;
		return {handle};
	}
	AllocatedBuffer GX::createBufferImpl(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memFlags) {
		ASSERT_MSG(bufferSize > 0, "Buffer size needs to be greater than 0!");

		AllocatedBuffer buf = {};
		buf._bufferSize = bufferSize;
		buf._vkUsageFlags = usageFlags;
		buf._vkMemoryPropertyFlags = memFlags;

		const VkBufferCreateInfo buffer_ci = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = bufferSize,
				.usage = usageFlags,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
		};
		VmaAllocationCreateInfo vmaAllocInfo = {};

		if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			vmaAllocInfo = {
					.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
					.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
					.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
			};
		}
		if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			// Check if coherent buffer is available.
			VK_CHECK(vkCreateBuffer(_backend.getDevice(), &buffer_ci, nullptr, &buf._vkBuffer));
			VkMemoryRequirements requirements = {};
			vkGetBufferMemoryRequirements(_backend.getDevice(), buf._vkBuffer, &requirements);
			vkDestroyBuffer(_backend.getDevice(), buf._vkBuffer, nullptr);
			buf._vkBuffer = VK_NULL_HANDLE;

			if (requirements.memoryTypeBits & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
				vmaAllocInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				buf._isCoherentMemory = true;
			}
		}
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaCreateBufferWithAlignment(_backend.getAllocator(), &buffer_ci, &vmaAllocInfo, 16, &buf._vkBuffer, &buf._vmaAllocation, nullptr);
		// handle memory-mapped buffers
		if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			vmaMapMemory(_backend.getAllocator(), buf._vmaAllocation, &buf._mappedPtr);
		}

		ASSERT_MSG(buf._vkBuffer != VK_NULL_HANDLE, "VkBuffer is VK_NULL_HANDLE after creation!");

		// shader access
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			const VkBufferDeviceAddressInfo buffer_device_ai = {
					.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
					.buffer = buf._vkBuffer,
			};
			buf._vkDeviceAddress = vkGetBufferDeviceAddress(_backend.getDevice(), &buffer_device_ai);
			ASSERT(buf._vkDeviceAddress);
		}
		return buf;
	}
	TextureHandle GX::createTexture(TextureSpec spec) {
		ASSERT(spec.usage);
		// resolve usage flags
		VkImageUsageFlags usage_flags = (spec.storage == StorageType::Device) ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0;

		if (spec.usage & TextureUsageBits::TextureUsageBits_Sampled) {
			usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (spec.usage & TextureUsageBits::TextureUsageBits_Storage) {
			ASSERT_MSG(spec.samples == SampleCount::X1, "Storage images cannot be multisampled!");
			usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		if (spec.usage & TextureUsageBits::TextureUsageBits_Attachment) {
			usage_flags |= (vkutil::IsFormatDepthOrStencil(spec.format)) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (spec.storage == StorageType::Memoryless) {
				usage_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			}
		}
		if (spec.storage != StorageType::Memoryless) {
			// for now, always set this flag so we can read it back
			usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (spec.numMipLevels == 0) {
			LOG_USER(LogType::Warning, "The number of mip levels specified must be greater than 0!");
			spec.numMipLevels = 1;
		}
		ASSERT_MSG(usage_flags != 0, "Invalid usage flags for texture creation!");
		ASSERT_MSG(spec.type == TextureType::Type_2D || spec.type == TextureType::Type_3D || spec.type == TextureType::Type_Cube, "Only 2D, 3D and Cube textures are supported.");

		// resolve sample count
		const VkSampleCountFlagBits sample_bits = toVulkan(spec.samples);
		// resolve memory flags
		const VkMemoryPropertyFlags mem_flags = StorageTypeToVkMemoryPropertyFlags(spec.storage);
		// resolve extent ig
		const VkExtent3D extent3D = { spec.dimension.width, spec.dimension.height, 1 };

		// resolve actions based on texture type
		uint32_t _numLayers = spec.numLayers;
		VkImageType _imagetype;
		VkImageViewType _imageviewtype;
		VkImageCreateFlags _imageCreateFlags = 0;
		switch (spec.type) {
			case TextureType::Type_2D:
				_imagetype = VK_IMAGE_TYPE_2D;
				_imageviewtype = (_numLayers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
				break;
			case TextureType::Type_3D:
				_imagetype = VK_IMAGE_TYPE_3D;
				_imageviewtype = VK_IMAGE_VIEW_TYPE_3D;
				break;
			case TextureType::Type_Cube:
				_imagetype = VK_IMAGE_TYPE_2D;
				_imageviewtype = (_numLayers > 1) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
				_numLayers *= 6;
				_imageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
				break;
			default:
				ASSERT_MSG(false, "Program should never reach this!");
		}

		AllocatedImage obj = createTextureImpl(usage_flags, mem_flags, extent3D, spec.format, _imagetype, _imageviewtype, 1, _numLayers, sample_bits, _imageCreateFlags);
		snprintf(obj._debugName, sizeof(obj._debugName) - 1, "%s", spec.debugName);
		TextureHandle handle = _texturePool.create(std::move(obj));
		// if we have some data we want to upload, do that
		_awaitingCreation = true;
		if (spec.data) {
			ASSERT(spec.dataNumMipLevels <= spec.numMipLevels);
			ASSERT(spec.type == TextureType::Type_2D || spec.type == TextureType::Type_Cube);
			TexRange range = {
					.dimensions = extent3D,
					.numLayers = static_cast<uint32_t>((spec.type == TextureType::Type_Cube) ? 6 : 1),
					.numMipLevels = spec.dataNumMipLevels
			};
			this->upload(handle, spec.data, range);
			if (spec.generateMipmaps) {
				this->generateMipmaps(handle);
			}
		}
		return {handle};
	}
	void GX::generateMipmaps(TextureHandle handle) {
		if (handle.empty()) {
			LOG_USER(LogType::Warning, "Generate mipmap request with empty handle!");
			return;
		}
		AllocatedImage* image = _texturePool.get(handle);
		if (image->_numLevels <= 1) {
			return;
		}
		ASSERT(image->_vkCurrentImageLayout != VK_IMAGE_LAYOUT_UNDEFINED);
		const VulkanImmediateCommands::CommandBufferWrapper& wrapper = _imm->acquire();
		image->generateMipmap(wrapper._cmdBuf);
		_imm->submit(wrapper);
	}
	void AllocatedImage::transitionLayout(VkCommandBuffer cmd, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange) {
		const VkImageLayout oldImageLayout = (_vkCurrentImageLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)
						? (isDepthAttachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
						: _vkCurrentImageLayout;
		if (newImageLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL) {
			newImageLayout = isDepthAttachment() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		vkutil::StageAccess src = vkutil::getPipelineStageAccess(oldImageLayout);
		vkutil::StageAccess dst = vkutil::getPipelineStageAccess(newImageLayout);

		if (isDepthAttachment() && _isResolveAttachment) {
			// https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#renderpass-resolve-operations
			src.stage |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dst.stage |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			src.access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			dst.access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		}
		vkutil::ImageMemoryBarrier2(cmd, _vkImage, src, dst, oldImageLayout, newImageLayout, subresourceRange);
		_vkCurrentImageLayout = newImageLayout;
	}
	void AllocatedImage::generateMipmap(VkCommandBuffer cmd) {
		// Check if device supports downscaling for color or depth/stencil buffer based on image format
		{
			const uint32_t formatFeatureMask = (VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT);
			const bool hardwareDownscalingSupported = (_vkFormatProperties.optimalTilingFeatures & formatFeatureMask) == formatFeatureMask;

			// FIXME: the warning is printing a void *
			if (!hardwareDownscalingSupported) {
				LOG_USER(LogType::Warning, "Doesn't support hardware downscaling of this image format: {}", (void*)_vkFormat);
				return;
			}
		}
		const VkFilter blitFilter = [](bool isDepthOrStencilFormat, bool imageFilterLinear) {
			if (isDepthOrStencilFormat) {
				return VK_FILTER_NEAREST;
			}
			if (imageFilterLinear) {
				return VK_FILTER_LINEAR;
			}
			return VK_FILTER_NEAREST;
		}(vkutil::IsFormatDepthOrStencil(_vkFormat), _vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

		const VkImageAspectFlags imageAspectFlags = vkutil::AspectMaskFromFormat(_vkFormat);
		const VkImageLayout originalImageLayout = _vkCurrentImageLayout;
		ASSERT(originalImageLayout != VK_IMAGE_LAYOUT_UNDEFINED);
		this->transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VkImageSubresourceRange{imageAspectFlags, 0, 1, 0, _numLayers});

		// now make the mipmaps
		for (uint32_t layer = 0; layer < _numLayers; ++layer) {
			int32_t mipWidth = (int32_t)_vkExtent.width;
			int32_t mipHeight = (int32_t)_vkExtent.height;

			for (uint32_t i = 1; i < _numLevels; ++i) {
				// 1: Transition the i-th level to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; it will be copied into from the (i-1)-th layer
				vkutil::ImageMemoryBarrier2(cmd,
										 _vkImage,
										 vkutil::StageAccess{.stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, .access = VK_ACCESS_2_NONE},
										 vkutil::StageAccess{.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .access = VK_ACCESS_2_TRANSFER_WRITE_BIT},
										 VK_IMAGE_LAYOUT_UNDEFINED,
										 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
										 VkImageSubresourceRange{imageAspectFlags, i, 1, layer, 1});

				const int32_t nextLevelWidth = (mipWidth > 1) ? (mipWidth / 2) : 1;
				const int32_t nextLevelHeight = (mipHeight > 1) ? (mipHeight / 2) : 1;

				const VkOffset3D srcOffsets[2] = {
						VkOffset3D{0, 0, 0},
						VkOffset3D{mipWidth, mipHeight, 1},
				};
				const VkOffset3D dstOffsets[2] = {
						VkOffset3D{0, 0, 0},
						VkOffset3D{nextLevelWidth, nextLevelHeight, 1},
				};
				const VkImageBlit blit = {
						.srcSubresource = VkImageSubresourceLayers{imageAspectFlags, i - 1, layer, 1},
						.srcOffsets = {srcOffsets[0], srcOffsets[1]},
						.dstSubresource = VkImageSubresourceLayers{imageAspectFlags, i, layer, 1},
						.dstOffsets = {dstOffsets[0], dstOffsets[1]},
				};
				vkCmdBlitImage(cmd,
							   _vkImage,
							   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							   _vkImage,
							   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   1,
							   &blit,
							   blitFilter);
				// 3: Transition i-th level to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL as it will be read from in the next iteration
				ImageMemoryBarrier2(cmd,
										 _vkImage,
										 vkutil::StageAccess{.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .access = VK_ACCESS_2_TRANSFER_WRITE_BIT},
										 vkutil::StageAccess{.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .access = VK_ACCESS_2_TRANSFER_READ_BIT},
										 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
										 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
										 VkImageSubresourceRange{imageAspectFlags, i, 1, layer, 1});

				mipWidth = nextLevelWidth;
				mipHeight = nextLevelHeight;
			}
		}

	}
	AllocatedImage GX::createTextureImpl(VkImageUsageFlags usageFlags,
										 VkMemoryPropertyFlags memFlags,
										 VkExtent3D extent3D,
										 VkFormat format,
										 VkImageType imageType,
										 VkImageViewType imageViewtype,
										 uint32_t numLevels,
										 uint32_t numLayers,
										 VkSampleCountFlagBits sampleCountFlagBits,
										 VkImageCreateFlags createFlags)
	{
		ASSERT_MSG(numLevels > 0, "The image must contain at least one mip-level");
		ASSERT_MSG(numLayers > 0, "The image must contain at least one layer");
		ASSERT(extent3D.width > 0);
		ASSERT(extent3D.height > 0);
		ASSERT(extent3D.depth > 0);

		VkImageCreateFlags image_cf = createFlags;
		const VkImageCreateInfo image_ci = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,

				.flags = image_cf,
				.imageType = imageType,
				.format = format,
				.extent = extent3D,
				.mipLevels = numLevels,
				.arrayLayers = numLayers,
				.samples = sampleCountFlagBits,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = usageFlags,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,

				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		VmaAllocationCreateInfo allocation_ci = {};
		allocation_ci.usage = (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_AUTO;

		AllocatedImage obj = {};
		VK_CHECK(vmaCreateImage(_backend.getAllocator(), &image_ci, &allocation_ci, &obj._vkImage, &obj._vmaAllocation, nullptr));
		obj._vkExtent = extent3D;
		obj._vkUsageFlags = usageFlags;
		obj._vkSampleCountFlagBits = sampleCountFlagBits;
		obj._vkImageType = imageType;
		obj._vkFormat = format;
		obj._numLayers = numLayers;
		obj._numLevels = numLevels;
		vkGetPhysicalDeviceFormatProperties(_backend.getPhysicalDevice(), obj._vkFormat, &obj._vkFormatProperties);

		// if memory is manually managed on host
		if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			vmaMapMemory(_backend.getAllocator(), obj._vmaAllocation, &obj._mappedPtr);
		}
		// create image views
		const VkImageAspectFlags aspectMask = vkutil::AspectMaskFromFormat(obj._vkFormat);
		const VkImageViewCreateInfo image_view_ci = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = nullptr,
				.image = obj._vkImage,
				.viewType = imageViewtype,
				.format = format,
				.subresourceRange = {
						.aspectMask = aspectMask,
						.baseMipLevel = 0,
						.levelCount = VK_REMAINING_MIP_LEVELS,
						.baseArrayLayer = 0,
						.layerCount = numLayers
				}
		};
		VK_CHECK(vkCreateImageView(_backend.getDevice(), &image_view_ci, nullptr, &obj._vkImageView));
		if (obj._vkUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) {
			VK_CHECK(vkCreateImageView(_backend.getDevice(), &image_view_ci, nullptr, &obj._vkImageViewStorage));
		}
		return obj;
	}

	SamplerHandle GX::createSampler(SamplerSpec spec) {
		VkFilter minfilter = toVulkan(spec.minFilter);
		VkFilter magfilter = toVulkan(spec.magFilter);
		VkSamplerAddressMode addressU = toVulkan(spec.wrapU);
		VkSamplerAddressMode addressV = toVulkan(spec.wrapV);
		VkSamplerAddressMode addressW = toVulkan(spec.wrapW);

		// creating sampler requires little work so we dont need an _Impl function for it
		VkSamplerCreateInfo info = {
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.pNext = nullptr,

				.magFilter = magfilter,
				.minFilter = minfilter,
				.addressModeU = addressU,
				.addressModeV = addressV,
				.addressModeW = addressW,

				.maxAnisotropy = 1.0f,
				.minLod = -1000,
				.maxLod = 1000,

				.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
				.unnormalizedCoordinates = false,
		};

		AllocatedSampler obj = {};
		vkCreateSampler(_backend.getDevice(), &info, nullptr, &obj._vkSampler);
		snprintf(obj.debugName, sizeof(obj.debugName) - 1, "%s", spec.debugName);
		SamplerHandle handle = _samplerPool.create(std::move(obj));
		_awaitingCreation = true;
		return {handle};
	}
	void GX::destroy(BufferHandle handle) {
		AllocatedBuffer* buf = _bufferPool.get(handle);
		if (!buf) {
			return;
		}

		if (buf->_mappedPtr) {
			vmaUnmapMemory(_backend.getAllocator(), buf->_vmaAllocation);
		}
		deferredTask(std::packaged_task<void()>([vma = _backend.getAllocator(), buffer = buf->_vkBuffer, allocation = buf->_vmaAllocation]() {
			vmaDestroyBuffer(vma, buffer, allocation);
		}));
		_bufferPool.destroy(handle);
	}
	void GX::destroy(TextureHandle handle) {
		AllocatedImage* image = _texturePool.get(handle);

		if (!image) {
			return;
		}
		deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), imageView = image->_vkImageView]() {
			vkDestroyImageView(device, imageView, nullptr);
		}));
		if (image->_vkImageViewStorage) {
			deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), imageView = image->_vkImageViewStorage]() {
				vkDestroyImageView(device, imageView, nullptr);
			}));
		}
		// necessary for swapchain imges which swapchain is created from
		if (!image->_isOwning) {
			_texturePool.destroy(handle);
			_awaitingCreation = true;
			return;
		}
		if (image->_mappedPtr) {
			vmaUnmapMemory(_backend.getAllocator(), image->_vmaAllocation);
		}
		deferredTask(std::packaged_task<void()>([vma = _backend.getAllocator(), image = image->_vkImage, allocation = image->_vmaAllocation]() {
			vmaDestroyImage(vma, image, allocation);
		}));
		_texturePool.destroy(handle);
		_awaitingCreation = true;
	}
	void GX::destroy(SamplerHandle handle) {
		AllocatedSampler sampler = *_samplerPool.get(handle);
		_samplerPool.destroy(handle);
		deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), sampler = sampler._vkSampler]() {
			vkDestroySampler(device, sampler, nullptr);
		}));
	}
	void GX::destroy(ShaderHandle handle) {
		VkShaderModule module = _shaderPool.get(handle)->_vkModule;
		vkDestroyShaderModule(_backend.getDevice(), module, nullptr);
		_shaderPool.destroy(handle);
	}
	MeshData GX::createMesh(const std::vector<Vertex>& vertices) {
		BufferHandle vertexHandle = this->createBuffer({
				.size = sizeof(Vertex) * vertices.size(),
				.usage = BufferUsageBits::BufferUsageBits_Storage,
				.storage = StorageType::Device,
				.data = vertices.data(),
		});
		MeshData mesh = {};
		mesh._vertexBuffer = vertexHandle;
		mesh._vertexCount = vertices.size();
		return mesh;
	}

	MeshData GX::createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		BufferHandle vertexHandle = this->createBuffer({
				.size = sizeof(Vertex) * vertices.size(),
				.usage = BufferUsageBits::BufferUsageBits_Storage,
				.storage = StorageType::Device,
				.data = vertices.data()
		});
		BufferHandle indexHandle = this->createBuffer({
				.size = sizeof(uint32_t) * indices.size(),
				.usage = BufferUsageBits::BufferUsageBits_Index,
				.storage = StorageType::Device,
				.data = indices.data()
		});
		MeshData mesh = {};
		mesh._vertexBuffer = vertexHandle;
		mesh._vertexCount = vertices.size();
		mesh._indexBuffer = indexHandle;
		mesh._indexCount = indices.size();
		return mesh;
	}

	ShaderHandle GX::createShader(ShaderSpec spec) {
		VkShaderModuleCreateInfo create_info = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		create_info.pCode = static_cast<uint32_t const*>(spec.spirvBlob->getBufferPointer());
		create_info.codeSize = spec.spirvBlob->getBufferSize();

		VkShaderModule shaderModule;
		VK_CHECK(vkCreateShaderModule(_backend.getDevice(), &create_info, nullptr, &shaderModule));
		// remember shader files should include both vert and frag stages so we can assign both to one
		ShaderData shader = {};
		shader._vkModule = std::move(shaderModule);
		shader.pushConstantSize = spec.pushConstantSize;
		return _shaderPool.create(std::move(shader));
	}
	void GX::checkAndUpdateDescriptorSets() {
		if (!_awaitingCreation) {
			return;
		}
		uint32_t newMaxTextures = _currentMaxTextureCount;
		uint32_t newMaxSamplers = _currentMaxSamplerCount;

		while (_texturePool._objects.size() > newMaxTextures) {
			newMaxTextures *= 2;
		}
		while (_samplerPool._objects.size() > newMaxSamplers) {
			newMaxSamplers *= 2;
		}
		if (newMaxTextures != _currentMaxTextureCount || newMaxSamplers != _currentMaxSamplerCount || _awaitingNewImmutableSamplers) {
			growDescriptorPool(newMaxTextures, newMaxSamplers);
		}

		// IMAGES //
		std::vector<VkDescriptorImageInfo> infoSampledImages;
		std::vector<VkDescriptorImageInfo> infoStorageImages;
		uint32_t numObjects = _texturePool.numObjects();
		infoSampledImages.reserve(numObjects);
		infoStorageImages.reserve(numObjects);

		VkImageView dummyImageView = _texturePool._objects[0]._obj._vkImageView;

		for (const auto& obj : _texturePool._objects) {
			const AllocatedImage& img = obj._obj;
			VkImageView view = obj._obj._vkImageView;
			VkImageView storageView = obj._obj._vkImageViewStorage ? obj._obj._vkImageViewStorage : view;
			// multisampled images cannot be directly accessed from shaders
			const bool isTextureAvailable = (img.getSampleCount() & VK_SAMPLE_COUNT_1_BIT) == VK_SAMPLE_COUNT_1_BIT;
			const bool isSampledImage = isTextureAvailable && img.isSampledImage();
			const bool isStorageImage = isTextureAvailable && img.isStorageImage();
			infoSampledImages.push_back(VkDescriptorImageInfo{
					.sampler = VK_NULL_HANDLE,
					.imageView = isSampledImage ? view : dummyImageView,
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			});
			LOG_USER(LogType::Info, "{} - {}", img._debugName, _texturePool.findObject(&obj._obj).index());
			ASSERT_MSG(infoSampledImages.back().imageView != VK_NULL_HANDLE, "Sampled imageView is null!");
			infoStorageImages.push_back(VkDescriptorImageInfo{
					.sampler = VK_NULL_HANDLE,
					.imageView = isStorageImage ? storageView : dummyImageView,
					.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			});
		}

		// SAMPLERS //
		std::vector<VkDescriptorImageInfo> infoSamplers;
		infoSamplers.reserve(_samplerPool._objects.size());

		VkSampler linearSampler = _samplerPool._objects[0]._obj._vkSampler;

		for (const auto& obj : _samplerPool._objects) {
			infoSamplers.push_back({
					.sampler = obj._obj._vkSampler ? obj._obj._vkSampler : linearSampler,
					.imageView = VK_NULL_HANDLE,
					.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			});
		}

		// now write to descriptor set //
		VkWriteDescriptorSet write[kNumBindlessBindings] = {};
		uint32_t numWrites = 0;

		if (!infoSampledImages.empty()) {
			write[numWrites++] = VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = _vkDSet,
					.dstBinding = kTextureBinding,
					.dstArrayElement = 0,
					.descriptorCount = (uint32_t)infoSampledImages.size(),
					.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
					.pImageInfo = infoSampledImages.data(),
			};
		}

		if (!infoSamplers.empty()) {
			write[numWrites++] = VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = _vkDSet,
					.dstBinding = kSamplerBinding,
					.dstArrayElement = 0,
					.descriptorCount = (uint32_t)infoSamplers.size(),
					.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
					.pImageInfo = infoSamplers.data(),
			};
		}

		if (!infoStorageImages.empty()) {
			write[numWrites++] = VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = _vkDSet,
					.dstBinding = kStorageImageBinding,
					.dstArrayElement = 0,
					.descriptorCount = (uint32_t)infoStorageImages.size(),
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
					.pImageInfo = infoStorageImages.data(),
			};
		}

		// update descriptor set
		if (numWrites) {
			_imm->wait(_imm->getLastSubmitHandle());
			vkUpdateDescriptorSets(_backend.getDevice(), numWrites, write, 0, nullptr);
		}
		_awaitingCreation = false;
	}

	void GX::growDescriptorPool(uint32_t newMaxTextureCount, uint16_t newMaxSamplerCount) {
		// update maxes
		_currentMaxTextureCount = newMaxTextureCount;
		_currentMaxSamplerCount = newMaxSamplerCount;

		const uint32_t MAX_TEXTURE_LIMIT = _backend.getPhysDevicePropertiesV12().maxDescriptorSetUpdateAfterBindSampledImages;
		ASSERT_MSG(newMaxTextureCount <= MAX_TEXTURE_LIMIT, "Max sampled textures exceeded: {}, but maximum of {} is allowed!", newMaxTextureCount, MAX_TEXTURE_LIMIT);

		const uint32_t MAX_SAMPLER_LIMIT = _backend.getPhysDevicePropertiesV12().maxDescriptorSetUpdateAfterBindSamplers;
		ASSERT_MSG(newMaxSamplerCount <= MAX_SAMPLER_LIMIT, "Max samplers exceeded: {}, but maximum of {} is allowed!", newMaxSamplerCount, MAX_SAMPLER_LIMIT);

		if (_vkDSL != VK_NULL_HANDLE) {
			deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), dsl = _vkDSL]() {
				vkDestroyDescriptorSetLayout(device, dsl, nullptr);
			}));
		}
		if (_vkDPool != VK_NULL_HANDLE) {
			deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), dp = _vkDPool]() {
				vkDestroyDescriptorPool(device, dp, nullptr);
			}));
		}

		VkShaderStageFlags stage_flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		const VkDescriptorSetLayoutBinding bindings[kNumBindlessBindings] = {
				VkDescriptorSetLayoutBinding(kTextureBinding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, newMaxTextureCount, stage_flags),
				VkDescriptorSetLayoutBinding(kSamplerBinding, VK_DESCRIPTOR_TYPE_SAMPLER, newMaxSamplerCount, stage_flags),
				VkDescriptorSetLayoutBinding(kStorageImageBinding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, newMaxTextureCount, stage_flags),
		};
		const uint32_t flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
		VkDescriptorBindingFlags bindingFlags[kNumBindlessBindings];
		for (uint32_t& bindingFlag : bindingFlags) {
			bindingFlag = flags;
		}
		const VkDescriptorSetLayoutBindingFlagsCreateInfo dsl_bf_ci = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
				.bindingCount = (uint32_t) kNumBindlessBindings,
				.pBindingFlags = bindingFlags,
		};
		const VkDescriptorSetLayoutCreateInfo dsl_ci = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext = &dsl_bf_ci,
				.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
				.bindingCount = (uint32_t) kNumBindlessBindings,
				.pBindings = bindings,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(_backend.getDevice(), &dsl_ci, nullptr, &_vkDSL));

		{
			const VkDescriptorPoolSize poolSizes[kNumBindlessBindings]{
					VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, newMaxTextureCount},
					VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, newMaxSamplerCount},
					VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, newMaxTextureCount},
			};
			const VkDescriptorPoolCreateInfo dp_ci = {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
					.maxSets = 1,
					.poolSizeCount = (uint32_t)kNumBindlessBindings,
					.pPoolSizes = poolSizes,
			};
			VK_CHECK(vkCreateDescriptorPool(_backend.getDevice(), &dp_ci, nullptr, &_vkDPool));
			const VkDescriptorSetAllocateInfo ds_ai = {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.descriptorPool = _vkDPool,
					.descriptorSetCount = 1,
					.pSetLayouts = &_vkDSL,
			};
			VK_CHECK(vkAllocateDescriptorSets(_backend.getDevice(), &ds_ai, &_vkDSet));
		}
		_awaitingNewImmutableSamplers = false;
	}

	PipelineHandle GX::createPipeline(PipelineSpec spec) {
		RenderPipeline pipeline = {};
		pipeline._spec = std::move(spec);
		PipelineHandle handle = _pipelinePool.create(std::move(pipeline));
		return {handle};
	}
	void GX::destroy(PipelineHandle handle) {
		RenderPipeline* rps = _pipelinePool.get(handle);
		if (!rps) {
			return;
		}
		deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), pipeline = rps->_vkPipeline]() {
			vkDestroyPipeline(device, pipeline, nullptr);
		}));
		deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), layout = rps->_vkPipelineLayout]() {
			vkDestroyPipelineLayout(device, layout, nullptr);
		}));
		_pipelinePool.destroy(handle);
	}
	void GX::bindDefaultDescriptorSets(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout) {
		const std::array<VkDescriptorSet, 4> descriptor_sets = {  _vkDSet, _vkDSet, _vkDSet, _vkGlobalDSet };
		vkCmdBindDescriptorSets(cmd, bindPoint, layout, 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);
	}
	RenderPipeline* GX::resolveRenderPipeline(PipelineHandle handle) {
		RenderPipeline* renderPipeline = _pipelinePool.get(handle);
		if (!renderPipeline) {
			LOG_USER(LogType::Warning, "Render pipeline does not exist, pass in a valid handle!");
			return VK_NULL_HANDLE;
		}
		// updating descriptor layout //
		if (renderPipeline->_vkLastDescriptorSetLayout != _vkDSL) {
			deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), pipeline = renderPipeline->_vkPipeline]() {
				vkDestroyPipeline(device, pipeline, nullptr);
			}));
			deferredTask(std::packaged_task<void()>([device = _backend.getDevice(), layout = renderPipeline->_vkPipelineLayout]() {
				vkDestroyPipelineLayout(device, layout, nullptr);
			}));
			renderPipeline->_vkPipeline = VK_NULL_HANDLE;
			renderPipeline->_vkLastDescriptorSetLayout = _vkDSL;
		}

		// RETURN EXISTING PIPELINE //
		if (renderPipeline->_vkPipeline != VK_NULL_HANDLE) {
			return renderPipeline;
		}
		// or, CREATE NEW PIPELINE //

		PipelineSpec& spec = renderPipeline->_spec;

		PipelineBuilder builder = {};
		builder.set_cull_mode(spec.cull);
		builder.set_polygon_mode(spec.polygon);
		builder.set_topology_mode(spec.topology);
		builder.set_multisampling_mode(spec.multisample);
		builder.set_blending_mode(spec.blend);

		builder.set_color_formats(spec.formats.colorFormats);
		builder.set_depth_format(spec.formats.depthFormat);

		VkShaderModule module = _shaderPool.get(spec.shaderhandle)->_vkModule;
		ASSERT_MSG(module, "Shader module not found!");
		builder.set_module(module);

		size_t pc_size = _shaderPool.get(spec.shaderhandle)->pushConstantSize;
		// PUSH CONSTANTS
		// use reflection to get the size of push constant from slang
		uint32_t pushConstantsSize = (pc_size != 0) ? pc_size : sizeof(GPU::PerObjectData); // TODO: push constant size resolving logic is horrible
		const VkPhysicalDeviceLimits& limits = _backend.getPhysDeviceProperties().limits;
		ASSERT_MSG(pushConstantsSize <= limits.maxPushConstantsSize, "Push constants size exceeded {} (max {} bytes)", pushConstantsSize, limits.maxPushConstantsSize);
		VkPushConstantRange range = {
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // push constant usable in both modules
				.offset = 0,
				.size = pushConstantsSize,
		};
		// DESCRIPTOR LAYOUT
		const std::array<VkDescriptorSetLayout, 4> dsls = { _vkDSL, _vkDSL, _vkDSL, _vkGlobalDSL };

		const VkPipelineLayoutCreateInfo pipeline_layout_info = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = static_cast<uint32_t>(dsls.size()),
				.pSetLayouts = dsls.data(),
				.pushConstantRangeCount = pushConstantsSize ? 1u : 0u,
				.pPushConstantRanges = pushConstantsSize ? &range : nullptr,
		};
		VkPipelineLayout piplineLayout = VK_NULL_HANDLE;
		VK_CHECK(vkCreatePipelineLayout(_backend.getDevice(), &pipeline_layout_info, nullptr, &piplineLayout));

		renderPipeline->_vkPipeline = builder.build(_backend.getDevice(), piplineLayout);
		renderPipeline->_vkPipelineLayout = piplineLayout;
		return renderPipeline;
	}

	void GX::deferredTask(std::packaged_task<void()>&& task, SubmitHandle handle) const {
		if (handle.empty()) {
			handle = _imm->getNextSubmitHandle();
		}
		_deferredTasks.emplace_back(std::move(task), handle);
	}
	void GX::processDeferredTasks() const {
		std::vector<DeferredTask>::iterator it = _deferredTasks.begin();

		while (it != _deferredTasks.end() && _imm->isReady(it->_handle, true)) {
			(it++)->_task();
		}
		_deferredTasks.erase(_deferredTasks.begin(), it);
	}
	void GX::waitDeferredTasks() {
		for (auto& task : _deferredTasks) {
			_imm->wait(task._handle);
			task._task();
		}
		_deferredTasks.clear();
	}
	bool ValidateRange(const VkExtent3D& extent3D, uint32_t numLevels, const TexRange& range) {
		if (range.dimensions.width <= 0 ||
			range.dimensions.height <= 0 ||
			range.dimensions.depth <= 0 ||
			range.numLayers <= 0 ||
			range.numMipLevels <= 0) {
			LOG_USER(LogType::Error, "Values like: width, height, depth, numLayers and mipLevel must all be at least greater than 0.");
			return false;
		}
		if (range.mipLevel > numLevels) {
			LOG_USER(LogType::Error, "Requested mipLevels exceed texture's mipLevels!");
			return false;
		}
		const uint32_t texWidth = std::max(extent3D.width >> range.mipLevel, 1u);
		const uint32_t texHeight = std::max(extent3D.height >> range.mipLevel, 1u);
		const uint32_t texDepth = std::max(extent3D.depth >> range.mipLevel, 1u);

		if (range.dimensions.width > texWidth ||
			range.dimensions.height > texHeight ||
			range.dimensions.depth > texDepth) {
			LOG_USER(LogType::Error, "Range dimensions exceed texture dimensions!");
			return false;
		}
		if (range.offset.x > texWidth - range.dimensions.width ||
			range.offset.y > texHeight - range.dimensions.height ||
			range.offset.z > texDepth - range.dimensions.depth) {
			LOG_USER(LogType::Error, "Range dimensions exceed texture dimensions when accounting for offsets!");
			return false;
		}
		return true;
	}
	void GX::upload(BufferHandle handle, const void* data, size_t size, size_t offset) {
		if (!data) {
			LOG_USER(LogType::Warning, "Attempting to upload data which is null!");
			return;
		}
		ASSERT_MSG(size > 0, "Size must be greater than 0!");

		AllocatedBuffer* buffer = _bufferPool.get(handle);

		if (offset + size > buffer->_bufferSize) {
			LOG_USER(LogType::Error, "Buffer request to upload is out of range! (Either the uploaded data size exceeds the size of the actual buffer or its offset is exceeding the total range)");
			return;
		}
		_staging->bufferSubData(*buffer, offset, size, data);
	}
	void GX::download(BufferHandle handle, void* data, size_t size, size_t offset) {
		if (!data) {
			LOG_USER(LogType::Warning, "Data is null");
			return;
		}
		AllocatedBuffer* buffer = _bufferPool.get(handle);

		if (!buffer) {
			LOG_USER(LogType::Error, "Retrieved buffer is null, handle must have been invalid!");
			return;
		}
		if (offset + size <= buffer->_bufferSize) {
			LOG_USER(LogType::Error, "Buffer request to download is out of range!");
			return;
		}
		buffer->getBufferSubData(*this, offset, size, data);
	}
	void GX::upload(TextureHandle handle, const void* data, const TexRange& range) {
		if (!data) {
			LOG_USER(LogType::Warning, "Attempting to upload data which is null!");
			return;
		}
		AllocatedImage* image = _texturePool.get(handle);
		ASSERT_MSG(image, "Attempting to use texture via invalid handle!");
		if (!ValidateRange(image->_vkExtent, image->_numLevels, range)) {
			LOG_USER(LogType::Warning, "Image failed validation check!");
		}
		// why is this here
		if (image->_vkImageType == VK_IMAGE_TYPE_3D) {
			_staging->imageData3D(
					*image,
					VkOffset3D{range.offset.x, range.offset.y, range.offset.z},
					VkExtent3D{range.dimensions.width, range.dimensions.height, range.dimensions.depth},
					image->_vkFormat,
					data);
		} else {
			const VkRect2D image_region = {
					.offset = {.x = range.offset.x, .y = range.offset.y},
					.extent = {.width = range.dimensions.width, .height = range.dimensions.height},
			};
			_staging->imageData2D(*image, image_region, range.mipLevel, range.numMipLevels, range.layer, range.numLayers, image->_vkFormat, data);
		}
	}
	void GX::download(TextureHandle handle, void* data, const TexRange &range) {
		if (!data) {
			LOG_USER(LogType::Warning, "Data is null.");
			return;
		}
		AllocatedImage* image = _texturePool.get(handle);
		if (!image) {
			LOG_USER(LogType::Error, "Retrieved image is null, handle must have been invalid!");
			return;
		}
		if (!ValidateRange(image->_vkExtent, image->_numLevels, range)) {
			LOG_USER(LogType::Warning, "Image validation failed!");
			return;
		}
		_staging->getImageData(*image,
							   VkOffset3D{range.offset.x, range.offset.y, range.offset.z},
							   VkExtent3D{range.dimensions.width, range.dimensions.height, range.dimensions.depth},
							   VkImageSubresourceRange{
									   .aspectMask = vkutil::AspectMaskFromFormat(image->_vkFormat),
									   .baseMipLevel = range.mipLevel,
									   .levelCount = range.numMipLevels,
									   .baseArrayLayer = range.layer,
									   .layerCount = range.numLayers,
							   },
							   image->_vkFormat,
							   data);
	}


	void AllocatedBuffer::bufferSubData(const GX& ctx, size_t offset, size_t size, const void* data) {
		// only host-visible buffers can be uploaded this way
		if (!_mappedPtr) {
			return;
		}
		ASSERT(offset + size <= _bufferSize);
		if (data) {
			memcpy((uint8_t*)_mappedPtr + offset, data, size);
		} else {
			memset((uint8_t*)_mappedPtr + offset, 0, size);
		}

		if (!_isCoherentMemory) {
			flushMappedMemory(ctx, offset, size);
		}
	}
	void AllocatedBuffer::getBufferSubData(const GX& ctx, size_t offset, size_t size, void* data) {
		// only host-visible buffers can be downloaded this way
		if (!_mappedPtr) {
			return;
		}
		ASSERT(offset + size <= _bufferSize);
		if (!_isCoherentMemory) {
			invalidateMappedMemory(ctx, offset, size);
		}
		memcpy(data, (const uint8_t*)_mappedPtr + offset, size);
	}
	void AllocatedBuffer::flushMappedMemory(const Slate::GX &ctx, VkDeviceSize offset, VkDeviceSize size) const {
		if (!isMapped()) {
			return;
		}
		vmaFlushAllocation(ctx._backend.getAllocator(), _vmaAllocation, offset, size);
	}
	void AllocatedBuffer::invalidateMappedMemory(const GX &ctx, VkDeviceSize offset, VkDeviceSize size) const {
		if (!isMapped()) {
			return;
		}
		vmaInvalidateAllocation(ctx._backend.getAllocator(), _vmaAllocation, offset, size);
	}
	TextureHandle GX::acquireCurrentSwapchainTexture() {
		return _swapchain->acquireCurrentImage();
	}
	const char* toString(VkImageLayout layout) {
		switch (layout) {
			case VK_IMAGE_LAYOUT_UNDEFINED:
				return "VK_IMAGE_LAYOUT_UNDEFINED";
			case VK_IMAGE_LAYOUT_GENERAL:
				return "VK_IMAGE_LAYOUT_GENERAL";
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				return "VK_IMAGE_LAYOUT_PREINITIALIZED";
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
				return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
				return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
				return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL";
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
				return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL";
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
				return "VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL";
			case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
				return "VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL";
			case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
				return "VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL";
			case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
				return "VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL";
			case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
				return "VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ";
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				return "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR";
			case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR";
			case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR";
			case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR";
			case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
				return "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR";
			case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
				return "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT";
			case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
				return "VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR";
			case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR";
			case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR";
			case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR";
			case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
				return "VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT";
			case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
				return "VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR";
			case VK_IMAGE_LAYOUT_MAX_ENUM:
				return "VK_IMAGE_LAYOUT_MAX_ENUM";
		}
	}
	const char* toString(VkFilter filter) {
		switch (filter) {
			case VK_FILTER_NEAREST:
				return "VK_FILTER_NEAREST";
			case VK_FILTER_LINEAR:
				return "VK_FILTER_LINEAR";
			case VK_FILTER_CUBIC_EXT:
				return "VK_FILTER_CUBIC";
			case VK_FILTER_MAX_ENUM:
				return "VK_FILTER_MAX_ENUM";
		}
	}
	// to make smart object happy in its header file
	// we only implement bufferhandle because we only use the smart holder type for the staging buffer
	void destroy(GX* gx, BufferHandle handle) {
		if (gx) {
			gx->destroy(handle);
		}
	}

}






