//
// Created by Hayden Rivas on 4/16/25.
//

#pragma once

#include "Slate/GXBackend.h"
#include "Slate/VkObjects.h"
#include "Slate/CommandBuffer.h"
#include "Slate/VulkanSwapchain.h"
#include "Slate/VulkanImmediateCommands.h"

#include "Slate/Common/Handles.h"
#include "Slate/Common/HelperMacros.h"
#include "Slate/PipelineBuilder.h"
#include "Slate/Resources/MeshResource.h"
#include "Slate/VK/vkenums.h"

#include "ResourcePool.h"
#include "Slate/Resources/TextureResource.h"
#include "Slate/SubmitHandle.h"
#include "Slate/Version.h"
#include "Slate/VulkanStagingDevice.h"
#include "SmartPointers.h"


#include <future>
#include <slang/slang-com-ptr.h>
#include <volk.h>

namespace Slate {
	struct DeferredTask
	{
		DeferredTask(std::packaged_task<void()>&& task, SubmitHandle handle) : _task(std::move(task)), _handle(handle) {}
		std::packaged_task<void()> _task;
		SubmitHandle _handle;
	};

	struct VulkanInstanceInfo
	{
		const char* app_name = "Unnamed_App";
		Version app_version;
		const char* engine_name = "Unnamed_Engine";
		Version engine_version;
	};


	// forward declare
	template<typename T>
	class SmartObject;

	// bits need to be uint8_t underlying type
	enum BufferUsageBits : uint8_t {
		BufferUsageBits_Index = 1 << 0,
		BufferUsageBits_Uniform = 1 << 1,
		BufferUsageBits_Storage = 1 << 2,
		BufferUsageBits_Indirect = 1 << 3
	};
	enum TextureUsageBits : uint8_t {
		TextureUsageBits_Sampled = 1 << 0,
		TextureUsageBits_Storage = 1 << 1,
		TextureUsageBits_Attachment = 1 << 2,
	};

	// strict enums can be whatever
	enum class StorageType : uint8_t {
		Device,
		HostVisible,
		Memoryless
	};

	struct TexRange
	{
		VkOffset3D offset = {};
		VkExtent3D dimensions = {1, 1, 1};

		uint32_t layer = 0;
		uint32_t numLayers = 1;
		uint32_t mipLevel = 0;
		uint32_t numMipLevels = 1;
	};

	// Specs should be low level but still a thin wrapper around the info creation processes need
	// User arguements will be even more abstract as they will not need to implement it via code
	struct SamplerSpec
	{
		SamplerFilter magFilter = SamplerFilter::Linear;
		SamplerFilter minFilter = SamplerFilter::Linear;
		SamplerWrap wrapU = SamplerWrap::Repeat;
		SamplerWrap wrapV = SamplerWrap::Repeat;
		SamplerWrap wrapW = SamplerWrap::Repeat;

		SamplerMip mipMap = SamplerMip::Disabled;
		bool anistrophic = true;
		const char* debugName = "Unknown Sampler";
	};
	struct BufferSpec
	{
		size_t size = 0;
		uint8_t usage = {};
		StorageType storage = StorageType::HostVisible;
		const void* data = nullptr;
		const char* debugName = "Unknown Buffer";
	};
	struct TextureSpec
	{
		VkExtent2D dimension = {};

		uint32_t numMipLevels = 1;
		uint32_t numLayers = 1;
		SampleCount samples = SampleCount::X1;

		uint8_t usage = {};
		StorageType storage = StorageType::Device;

		TextureType type = TextureType::Type_2D;
		VkFormat format = VK_FORMAT_UNDEFINED;

		const void* data = nullptr;
		uint32_t dataNumMipLevels = 1; // how many mip-levels we want to upload
		bool generateMipmaps = false;
		const char* debugName = "Unknown Texture";
	};
	struct PipelineSpec
	{
		TopologyMode topology = TopologyMode::TRIANGLE;
		PolygonMode polygon = PolygonMode::FILL;
		BlendingMode blend = BlendingMode::OFF;
		CullMode cull = CullMode::BACK;
		SampleCount multisample = SampleCount::X1;

		struct AttachmentFormats {
			// span is more limiting with syntax
			std::vector<VkFormat> colorFormats;
			VkFormat depthFormat;
		} formats = {};

		InternalShaderHandle shaderhandle;
	};
	struct RenderPipeline
	{
		PipelineSpec _spec;

		VkPipeline _vkPipeline = VK_NULL_HANDLE;
		VkPipelineLayout _vkPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout _vkLastDescriptorSetLayout = VK_NULL_HANDLE;
	};
	struct ShaderSpec
	{
		Slang::ComPtr<slang::IBlob> spirvBlob;
		size_t pushConstantSize;
	};
	struct ShaderData
	{
		size_t pushConstantSize;
		VkShaderModule _vkModule;
	};

	struct RGB
	{
		float r, g, b;
		VkClearColorValue getAsVkClearColorValue() const {
			return { r, g, b, 1};
		}
	};
	struct RGBA
	{
		float r, g, b, a;
		VkClearColorValue getAsVkClearColorValue() const {
			return { r, g, b, a};
		}
	};

	constexpr uint16_t kMaxColorAttachments = 16;
	struct RenderPass
	{
		struct ColorAttachmentDesc {
			InternalTextureHandle texture;
			InternalTextureHandle resolveTexture = {};
			ResolveMode resolveMode = ResolveMode::AVERAGE;
			LoadOperation loadOp = LoadOperation::NO_CARE;
			StoreOperation storeOp = StoreOperation::NO_CARE;
			Optional<RGBA> clear = std::nullopt;
		};
		struct DepthAttachmentDesc {
			InternalTextureHandle texture;
			InternalTextureHandle resolveTexture = {};
			ResolveMode resolveMode = ResolveMode::AVERAGE;
			LoadOperation loadOp = LoadOperation::NO_CARE;
			StoreOperation storeOp = StoreOperation::NO_CARE;
			Optional<float> clear = std::nullopt;
		};

		ColorAttachmentDesc color[kMaxColorAttachments];
		DepthAttachmentDesc depth;
	};

	struct Framebuffer
	{
		struct FramebufferDesc {
			InternalTextureHandle texture;
			InternalTextureHandle resolveTexture;
		};
		std::vector<FramebufferDesc> colorAttachments;
		FramebufferDesc depthAttachment;
	};

	// necessary between rendering calls



	struct ImGuiRequiredData
	{
		VkDevice device;
		VkInstance instance;
		VkPhysicalDevice physdevice;
		VkQueue graphicsqueue;
	};



	// forward declare
	class Vertex;

	class GX final {
	public:
		GX() = default;
		~GX() = default;

		void create(VulkanInstanceInfo info, GLFWwindow* glfWwindow);
		void destroy();

		void resizeSwapchain(int w, int h);
		VkSampler getLinearSampler() const { return _samplerPool.get(_linearSamplerHandle)->_vkSampler; }
	public:
		inline void deviceWaitIdle() const { vkDeviceWaitIdle(_backend.getDevice()); }
		// swapchain
		InternalTextureHandle acquireCurrentSwapchainTexture();

		inline uint32_t getFrameNum() const { return _swapchain->_currentFrameNum; }
		inline bool isSwapchainDirty() const { return _swapchain->isDirty(); }
		inline VkExtent2D getSwapchainExtent() const { return _swapchain->_vkExtent2D; };
		inline VkImage getCurrentSwapchainImage() const { return _swapchain->getCurrentImage(); }

		CommandBuffer& acquireCommand();
		void submitCommand(CommandBuffer& cmd, InternalTextureHandle texture = {});

		// helpers
		void generateMipmaps(InternalTextureHandle handle);
		VkDeviceAddress gpuAddress(InternalBufferHandle handle, size_t offset = 0);

		VkImageLayout getTextureCurrentLayout(InternalTextureHandle handle) const { return _texturePool.get(handle)->_vkCurrentImageLayout; }
		VkFormat getTextureFormat(InternalTextureHandle handle) const { return _texturePool.get(handle)->_vkFormat; };
		VkExtent2D getTextureExtent(InternalTextureHandle handle) const { VkExtent3D extent = _texturePool.get(handle)->_vkExtent; return {extent.width, extent.height}; }
		VkImage getTextureImage(InternalTextureHandle handle) const { return _texturePool.get(handle)->_vkImage; }
		VkImageView getTextureImageView(InternalTextureHandle handle) const { return _texturePool.get(handle)->_vkImageView; }
		inline AllocatedImage* getTexture(InternalTextureHandle handle) { return _texturePool.get(handle); }

		RenderPipeline*resolveRenderPipeline(InternalPipelineHandle handle);
		inline AllocatedBuffer* getAllocatedBuffer(InternalBufferHandle handle) { return _bufferPool.get(handle); };
		// this is literally only for the cmd buffer single function, so useless
		inline RenderPipeline& getPipelineObject(InternalPipelineHandle handle) { return *_pipelinePool.get(handle); }
	public:
		// confusing things
		void bindDefaultDescriptorSets(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipelineLayout layout);
		void checkAndUpdateDescriptorSets();
		void growDescriptorPool(uint32_t newMaxTextureCount, uint16_t newMaxSamplerCount);

		// execute a task some time in the future after the submit handle finished processing
		void deferredTask(std::packaged_task<void()>&& task, SubmitHandle handle = SubmitHandle()) const;
		void processDeferredTasks() const;
		void waitDeferredTasks();

		RGBA _clearColor = { 0.1, 0.1, 0.1, 1 };


		void upload(InternalBufferHandle handle, const void* data, size_t size, size_t offset = 0);
		void download(InternalBufferHandle handle, void* data, size_t size, size_t offset);

		void upload(InternalTextureHandle handle, const void* data, const TexRange& range);
		void download(InternalTextureHandle handle, void* data, const TexRange& range);
		InternalBufferHandle _globalBufferHandle;
	private:
		InternalSamplerHandle _linearSamplerHandle;
		InternalSamplerHandle _nearestSamplerHandle;
		InternalTextureHandle _dummyTextureHandle;

		mutable std::vector<DeferredTask> _deferredTasks;

		bool _awaitingCreation = false;
		bool _awaitingNewImmutableSamplers = false;
		uint32_t _currentMaxTextureCount = 16;
		uint16_t _currentMaxSamplerCount = 16;

		VkDescriptorSetLayout _vkGlobalDSL = VK_NULL_HANDLE;
		VkDescriptorPool _vkGlobalDPool = VK_NULL_HANDLE;
		VkDescriptorSet _vkGlobalDSet = VK_NULL_HANDLE;

		VkDescriptorSetLayout _vkDSL = VK_NULL_HANDLE;
		VkDescriptorPool _vkDPool = VK_NULL_HANDLE;
		VkDescriptorSet _vkDSet = VK_NULL_HANDLE;

		VkSemaphore _timelineSemaphore = VK_NULL_HANDLE;
		CommandBuffer _currentCommandBuffer;

		UniquePtr<VulkanSwapchain> _swapchain = nullptr;
		UniquePtr<VulkanStagingDevice> _staging = nullptr;
	public:
		GXBackend _backend;
		UniquePtr<VulkanImmediateCommands> _imm = nullptr;

		// both buffer and texture creation logic can be a little lengthy so we split it into two parts
		InternalBufferHandle createBuffer(BufferSpec spec);
		AllocatedBuffer createBufferImpl(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memFlags);

		InternalTextureHandle createTexture(TextureSpec spec);
		AllocatedImage createTextureImpl(VkImageUsageFlags usageFlags,
										 VkMemoryPropertyFlags memFlags,
										 VkExtent3D extent3D,
										 VkFormat format,
										 VkImageType imageType,
										 VkImageViewType imageViewType,
										 uint32_t numLevels,
										 uint32_t numLayers,
										 VkSampleCountFlagBits sampleCountFlagBits,
										 VkImageCreateFlags createFlags = 0);

		InternalSamplerHandle createSampler(SamplerSpec spec);
		InternalPipelineHandle createPipeline(PipelineSpec spec);
		InternalShaderHandle createShader(ShaderSpec spec);

		MeshData createMesh(const std::vector<Vertex>& vertices);
		MeshData createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void destroy(InternalBufferHandle handle);
		void destroy(InternalTextureHandle handle);
		void destroy(InternalSamplerHandle handle);
		void destroy(InternalPipelineHandle handle);
		void destroy(InternalShaderHandle handle);
	private:
		HandlePool<InternalShaderHandle, ShaderData> _shaderPool;
		HandlePool<InternalTextureHandle, AllocatedImage> _texturePool;
		HandlePool<InternalBufferHandle, AllocatedBuffer> _bufferPool;
		HandlePool<InternalSamplerHandle, AllocatedSampler> _samplerPool;
		HandlePool<InternalPipelineHandle, RenderPipeline> _pipelinePool;
		// these two may be closely intertwined

		friend class VulkanActions;
		friend class CommandBuffer;
		friend class AllocatedBuffer;
		friend class VulkanSwapchain;
		friend class VulkanStagingDevice;
		friend class VulkanImmediateCommands;
	public:
		// EDITOR ONLY
		inline ImGuiRequiredData requestImGuiRequiredData() const {
			return {
					.device = _backend.getDevice(),
					.instance = _backend.getInstance(),
					.physdevice = _backend.getPhysicalDevice(),
					.graphicsqueue = _backend.getGraphicsQueue()
			};
		};
		inline std::pair<VkSampler, VkImageView> requestViewportImageData(InternalTextureHandle texturehandle) {
			return {_samplerPool.get(_linearSamplerHandle)->_vkSampler, _texturePool.get(texturehandle)->_vkImageView};
		}
	};
}