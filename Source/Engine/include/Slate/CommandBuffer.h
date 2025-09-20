//
// Created by Hayden Rivas on 4/15/25.
//

#pragma once
#include <volk.h>

#include "Slate/VK/vkenums.h"
#include "Slate/Common/Handles.h"
#include "VulkanImmediateCommands.h"

namespace Slate {
	// forward declarations
	class GX;
	class Framebuffer;
	class RenderPass;

	// we only use it for the cmdBeginRendering command anyways
	struct Dependencies {
		enum { kMaxSubmitDependencies = 4 };
		InternalTextureHandle textures[kMaxSubmitDependencies] = {};
		InternalBufferHandle buffers[kMaxSubmitDependencies] = {};
	};

	struct DepthState {
		CompareOperation compareOp = CompareOperation::CompareOp_AlwaysPass;
		bool isDepthWriteEnabled = false;
	};



	// lets RAII this guy
	class CommandBuffer final {
	public:
		CommandBuffer() = default;
		explicit CommandBuffer(GX* gx);
		~CommandBuffer();

		VkCommandBufferSubmitInfo requestSubmitInfo() const;
		VkCommandBuffer requestVkCmdBuffer() const { return _wrapper ? _wrapper->_cmdBuf : VK_NULL_HANDLE; }
	public:
		void cmdBeginRendering(RenderPass pass, const Dependencies& deps = {});
		void cmdEndRendering();

		void cmdBindRenderPipeline(InternalPipelineHandle handle);
		void cmdBindIndexBuffer(InternalBufferHandle buffer);
		// we can pass in structs of any type for push constants!!
		// make sure it is mirrored on the shader code
		void cmdPushConstants(const void* data, uint32_t size, uint32_t offset);
		template<class Struct>
		void cmdPushConstants(const Struct& type, uint32_t offset = 0) {
			cmdPushConstants(&type, (uint32_t)sizeof(Struct), offset);
		}
		void cmdUpdateBuffer(InternalBufferHandle bufhandle, size_t offset, size_t size, const void* data);
		template<typename Struct>
		void cmdUpdateBuffer(InternalBufferHandle buffer, const Struct& data, size_t bufferOffset = 0) {
			cmdUpdateBuffer(buffer, bufferOffset, sizeof(Struct), &data);
		}

		void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t baseInstance = 0);
		void cmdDrawInstanced();
		void cmdDrawIndexedInstanced();


		void cmdSetViewport(VkExtent2D extent2D);
		void cmdSetScissor(VkExtent2D extent2D);

		void cmdBindDepthState(const DepthState& state);
		void cmdSetDepthBiasEnable(bool enable);
		void cmdSetDepthBias(float constantFactor, float slopeFactor, float clamp);


		void cmdTransitionLayout(InternalTextureHandle source, VkImageLayout newLayout);
		void cmdTransitionLayout(InternalTextureHandle source, VkImageLayout currentLayout, VkImageLayout newLayout);
		void cmdTransitionSwapchainLayout(VkImageLayout newLayout);

		void cmdCopyImageToBuffer(InternalTextureHandle source, InternalBufferHandle destination, const VkBufferImageCopy& region);
		void cmdCopyImage(InternalTextureHandle source, InternalTextureHandle destination, VkExtent2D size);
		void cmdBlitImage(InternalTextureHandle source, InternalTextureHandle destination);
		void _cmdBlitImage(InternalTextureHandle source, InternalTextureHandle destination, VkExtent2D srcSize, VkExtent2D dstSize);
		void cmdBlitToSwapchain(InternalTextureHandle source);
	private:
		void bufferBarrier(InternalBufferHandle bufhandle, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage);

	private:
		GX* _gxCtx = nullptr;
		const VulkanImmediateCommands::CommandBufferWrapper* _wrapper = nullptr;

		bool _isRecording = false; // cmdBegin
		bool _isRendering = false; // cmdBeginRendering

		VkPipeline _lastBoundPipeline = VK_NULL_HANDLE;
		InternalPipelineHandle _currentPipeline;

		SubmitHandle _lastSubmitHandle = {};
		friend class GX; // for injection of command buffer data
	};

}