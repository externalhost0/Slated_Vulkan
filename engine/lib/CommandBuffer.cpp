//
// Created by Hayden Rivas on 4/15/25.
//
#include "Slate/CommandBuffer.h"
#include "Slate/Common/HelperMacros.h"
#include "Slate/Common/Logger.h"
#include "Slate/GX.h"
#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkutil.h"

#include <volk.h>
namespace Slate {
	CommandBuffer::CommandBuffer(GX* gx) : _gxCtx(gx), _wrapper(&gx->_imm->acquire()) {};
	CommandBuffer::~CommandBuffer() {
		ASSERT_MSG(!_isRendering, "Please call to end rendering before destroying a Command Buffer!");
	}

	VkCommandBufferSubmitInfo CommandBuffer::requestSubmitInfo() const {
		VkCommandBufferSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBuffer = _wrapper->_cmdBuf;
		info.deviceMask = 0;
		return info;
	}

	void CommandBuffer::cmdBeginRendering(RenderPass pass, const Dependencies& deps) {
		ASSERT_MSG(!_isRendering, "Command Buffer is already rendering!");

		// some states that we will need to refrence throughout this function
		uint32_t n = 0;
		while (n < kMaxColorAttachments && pass.color[n].texture.valid()) {
			n++;
		}
		const uint32_t colorAttachmentCount = n;

		// STEP 1 //
		// TRANSITION ATTACHMENTS TO BE RENDERED TO //
//		{
//			// COLOR
//			for (int i = 0; i < colorAttachmentCount; ++i) {
//				TextureHandle handle = pass.color[i].texture;
//				if (handle.valid()) {
//					cmdTransitionLayout(handle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//				}
//				TextureHandle resolveHandle = pass.color[i].resolveTexture;
//				if (resolveHandle.valid()) {
//					_gxCtx->_texturePool.get(resolveHandle)->_isResolveAttachment = true;
//					cmdTransitionLayout(resolveHandle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//				}
//			}
//			// DEPTH
//			if (pass.depth.texture.valid()) {
//				AllocatedImage* texture = _gxCtx->getTexture(pass.depth.texture);
//				ASSERT_MSG(texture->_vkFormat != VK_FORMAT_UNDEFINED, "Depth texture has invalid format!");
//				cmdTransitionLayout(pass.depth.texture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//
//				if (pass.depth.resolveTexture.valid()) {
//					AllocatedImage* resolveTexture = _gxCtx->getTexture(pass.depth.resolveTexture);
//					ASSERT_MSG(texture->_vkFormat != VK_FORMAT_UNDEFINED, "Depth resolve texture has invalid format!");
//					resolveTexture->_isResolveAttachment = true;
//					cmdTransitionLayout(pass.depth.resolveTexture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//				}
//			}
//		}
		// STEP 2 //
		// MAKE NECESSARY OBJECTS INTO SHADER READ LAYOUT //
		for (uint32_t i = 0; i != Dependencies::kMaxSubmitDependencies && deps.textures[i].valid(); i++) {
			const AllocatedImage* texture = _gxCtx->getTexture(deps.textures[i]);
			ASSERT_MSG(!texture->_isSwapchainImage, "Dependency textures can not include swapchain images!");
			if (texture->_vkSampleCountFlagBits == VK_SAMPLE_COUNT_1_BIT) {
				cmdTransitionLayout(deps.textures[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}

		// STEP 3 //
		// CREATE RENDER ATTACMENT INFO //
		// make rendering acttachment info for colors
		VkRenderingAttachmentInfo colorAttachmentInfos[kMaxColorAttachments] = {};
		for (int i = 0; i < colorAttachmentCount; ++i) {
			const RenderPass::ColorAttachmentDesc& attachmentdesc = pass.color[i];
			VkImageView colorTexture = _gxCtx->getTextureImageView(attachmentdesc.texture);
			VkClearColorValue clear = attachmentdesc.clear->getAsVkClearColorValue();
			if (attachmentdesc.resolveTexture.valid()) {
				VkImageView resolveColorTexture = _gxCtx->getTextureImageView(attachmentdesc.resolveTexture);
				colorAttachmentInfos[i] = vkinfo::CreateColorAttachmentInfo(colorTexture,
																			(attachmentdesc.clear.has_value()) ? &clear : nullptr,
																			toVulkan(attachmentdesc.loadOp),
																			toVulkan(attachmentdesc.storeOp),
																			resolveColorTexture,
																			toVulkan(attachmentdesc.resolveMode));
			} else {
				colorAttachmentInfos[i] = vkinfo::CreateColorAttachmentInfo(colorTexture,
																			(attachmentdesc.clear.has_value()) ? &clear : nullptr,
																			toVulkan(attachmentdesc.loadOp),
																			toVulkan(attachmentdesc.storeOp));
			}
		}
		// make rendering attachment info for depth
		VkRenderingAttachmentInfo depthAttachmentInfo = {};
		const bool hasDepth = pass.depth.texture.valid();
		if (hasDepth) {
			VkClearDepthStencilValue clear = {};
			if (pass.depth.clear.has_value()) {
				clear = { pass.depth.clear.value(), 0 };
			}
			if (pass.depth.resolveTexture.valid()) {
				VkImageView resolveDepthTexture = _gxCtx->getTextureImageView(pass.depth.resolveTexture);
				depthAttachmentInfo = vkinfo::CreateDepthStencilAttachmentInfo(_gxCtx->getTextureImageView(pass.depth.texture),
																			   (pass.depth.clear.has_value()) ? &clear : nullptr,
																			   toVulkan(pass.depth.loadOp),
																			   toVulkan(pass.depth.storeOp),
																			   resolveDepthTexture,
																			   toVulkan(pass.depth.resolveMode));
			} else {
				depthAttachmentInfo = vkinfo::CreateDepthStencilAttachmentInfo(_gxCtx->getTextureImageView(pass.depth.texture),
																			   (pass.depth.clear.has_value()) ? &clear : nullptr,
																			   toVulkan(pass.depth.loadOp),
																			   toVulkan(pass.depth.storeOp));
			}
		}

		// all the attachments need to be the same size anyways so just use the first texture
		VkExtent2D renderExtent = _gxCtx->getTextureExtent(pass.color[0].texture);
		VkRenderingInfo rendering_i = {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext = nullptr,
				.flags = 0,
				.renderArea = {
						.offset = { 0, 0 },
						.extent = renderExtent
				},
				.layerCount = 1,
				.viewMask = 0,

				.colorAttachmentCount = colorAttachmentCount,
				.pColorAttachments = colorAttachmentInfos,
				.pDepthAttachment = hasDepth ? &depthAttachmentInfo : nullptr,
				.pStencilAttachment = nullptr
		};

		cmdSetScissor(renderExtent);
		cmdSetViewport(renderExtent);
		cmdBindDepthState({});

		_gxCtx->checkAndUpdateDescriptorSets();

		vkCmdSetDepthCompareOp(_wrapper->_cmdBuf, VK_COMPARE_OP_ALWAYS);
		vkCmdSetDepthBiasEnable(_wrapper->_cmdBuf, VK_FALSE);
		vkCmdBeginRendering(_wrapper->_cmdBuf, &rendering_i);
		_isRendering = true;
	}
	void CommandBuffer::cmdEndRendering() {
		vkCmdEndRendering(_wrapper->_cmdBuf);
		_isRendering = false;
	}

	void CommandBuffer::cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		vkCmdDraw(_wrapper->_cmdBuf, vertexCount, instanceCount, firstVertex, firstInstance);
	}
	void CommandBuffer::cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t baseInstance) {
		vkCmdDrawIndexed(_wrapper->_cmdBuf, indexCount, instanceCount, firstIndex, vertexOffset, baseInstance);
	}
	void CommandBuffer::cmdDrawInstanced() {

	}
	void CommandBuffer::cmdDrawIndexedInstanced() {

	}


	void CommandBuffer::cmdBindIndexBuffer(InternalBufferHandle handle) {
		AllocatedBuffer* buffer = _gxCtx->getAllocatedBuffer(handle);
		vkCmdBindIndexBuffer(_wrapper->_cmdBuf, buffer->_vkBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::cmdSetViewport(VkExtent2D extent2D) {
		// we flip the viewport because Vulkan is reversed using LH instead of OpenGL's RH
		// HOWEVER: using Slang compilier option to reflect the y axis solves this so we dont have to flip here
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(extent2D.width);
		viewport.height = static_cast<float>(extent2D.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(_wrapper->_cmdBuf, 0, 1, &viewport);
	}
	void CommandBuffer::cmdSetScissor(VkExtent2D extent2D) {
		VkRect2D scissor = {};
		scissor.offset.x = 0; // default 0 for both
		scissor.offset.y = 0;
		scissor.extent = extent2D;
		vkCmdSetScissor(_wrapper->_cmdBuf, 0, 1, &scissor);
	}
	void CommandBuffer::cmdTransitionLayout(InternalTextureHandle source, VkImageLayout newLayout) {
		cmdTransitionLayout(source, _gxCtx->getTextureCurrentLayout(source), newLayout);
	}
	void CommandBuffer::cmdTransitionLayout(InternalTextureHandle source, VkImageLayout currentLayout, VkImageLayout newLayout) {

		vkutil::StageAccess srcStage = vkutil::getPipelineStageAccess(currentLayout);
		vkutil::StageAccess dstStage = vkutil::getPipelineStageAccess(newLayout);

		if (_gxCtx->_texturePool.get(source)->_isResolveAttachment && vkutil::IsFormatDepthOrStencil(_gxCtx->getTextureFormat(source))) {
			// https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#renderpass-resolve-operations
			srcStage.stage |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			srcStage.stage |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage.access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			dstStage.access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		}

		VkImageMemoryBarrier2 barrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = srcStage.stage,
				.srcAccessMask = srcStage.access,

				.dstStageMask = dstStage.stage,
				.dstAccessMask = dstStage.access,

				.oldLayout = currentLayout,
				.newLayout = newLayout,

				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

				.image = _gxCtx->getTextureImage(source),
				.subresourceRange = vkinfo::CreateImageSubresourceRange(vkutil::AspectMaskFromFormat(_gxCtx->getTextureFormat(source)))
		};
		VkDependencyInfo dependency_i = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &barrier
		};
		vkCmdPipelineBarrier2(_wrapper->_cmdBuf, &dependency_i);
		// set the texture to its new image layout
		_gxCtx->_texturePool.get(source)->_vkCurrentImageLayout = newLayout;
	}
	void CommandBuffer::cmdBlitImage(InternalTextureHandle source, InternalTextureHandle destination) {
		if (_gxCtx->getTextureCurrentLayout(source) != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			this->cmdTransitionLayout(source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}
		if (_gxCtx->getTextureCurrentLayout(destination) != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			this->cmdTransitionLayout(destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}
		_cmdBlitImage(source, destination, _gxCtx->getTextureExtent(source), _gxCtx->getTextureExtent(destination));
	}
	void CommandBuffer::_cmdBlitImage(InternalTextureHandle source, InternalTextureHandle destination, VkExtent2D srcSize, VkExtent2D dstSize) {
		VkImageBlit2 blitRegion = { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

		blitRegion.srcOffsets[1].x = static_cast<int32_t>(srcSize.width);
		blitRegion.srcOffsets[1].y = static_cast<int32_t>(srcSize.height);
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = static_cast<int32_t>(dstSize.width);
		blitRegion.dstOffsets[1].y = static_cast<int32_t>(dstSize.height);
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo = { .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
		blitInfo.srcImage = _gxCtx->getTextureImage(source);
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.dstImage = _gxCtx->getTextureImage(destination);
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2KHR(_wrapper->_cmdBuf, &blitInfo);
	}
	void CommandBuffer::cmdCopyImage(InternalTextureHandle source, InternalTextureHandle destination, VkExtent2D size) {
		VkImageCopy2 copyRegion = { .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2, .pNext = nullptr };

		copyRegion.extent.width = static_cast<uint32_t>(size.width);
		copyRegion.extent.height = static_cast<uint32_t>(size.height);
		copyRegion.extent.depth = 1;

		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcSubresource.mipLevel = 0;

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstSubresource.mipLevel = 0;

		VkCopyImageInfo2 copyinfo = { .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2, .pNext = nullptr };
		copyinfo.dstImage = _gxCtx->getTextureImage(destination);
		copyinfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copyinfo.srcImage = _gxCtx->getTextureImage(source);
		copyinfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		copyinfo.regionCount = 1;
		copyinfo.pRegions = &copyRegion;

		vkCmdCopyImage2KHR(_wrapper->_cmdBuf, &copyinfo);
	}
	void CommandBuffer::cmdSetDepthBiasEnable(bool enable) {
		vkCmdSetDepthBiasEnable(_wrapper->_cmdBuf, enable ? VK_TRUE : VK_FALSE);
	}
	void CommandBuffer::cmdSetDepthBias(float constantFactor, float slopeFactor, float clamp) {
		vkCmdSetDepthBias(_wrapper->_cmdBuf, constantFactor, clamp, slopeFactor);
	}
	void CommandBuffer::cmdBindDepthState(const DepthState& state) {
		// https://github.com/corporateshark/lightweightvk/blob/master/lvk/vulkan/VulkanClasses.cpp#L2458
		const VkCompareOp op = toVulkan(state.compareOp);
		vkCmdSetDepthWriteEnable(_wrapper->_cmdBuf, state.isDepthWriteEnabled ? VK_TRUE : VK_FALSE);
		vkCmdSetDepthTestEnable(_wrapper->_cmdBuf, (op != VK_COMPARE_OP_ALWAYS || state.isDepthWriteEnabled) ? VK_TRUE : VK_FALSE);
		vkCmdSetDepthCompareOp(_wrapper->_cmdBuf, op);
	}

	void CommandBuffer::cmdPushConstants(const void* data, uint32_t size, uint32_t offset) {
		ASSERT_MSG(size % 4 == 0, "Push constant size needs to be a multiple of 4. Is size {}", size);
		const VkPhysicalDeviceLimits& limits = _gxCtx->_backend.getPhysDeviceProperties().limits;
		if (size + offset > limits.maxPushConstantsSize) {
			LOG_USER(LogType::Error, "Push constants size exceeded %u (max %u bytes)", size + offset, limits.maxPushConstantsSize);
		}

		if (_currentPipeline.empty()) {
			LOG_USER(LogType::Warning, "No pipeline currently bound, cannot perform push constants!");
			return;
		}

		const RenderPipeline& pipeline = _gxCtx->getPipelineObject(_currentPipeline);
		vkCmdPushConstants(_wrapper->_cmdBuf, pipeline._vkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, offset, size, data);
	}

	void CommandBuffer::cmdTransitionSwapchainLayout(VkImageLayout newLayout) {
		VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		barrier.oldLayout = _gxCtx->_swapchain->getCurrentImageLayout();
		barrier.newLayout = newLayout;
		barrier.subresourceRange = vkinfo::CreateImageSubresourceRange(vkutil::AspectMaskFromAttachmentLayout(newLayout));
		barrier.image = _gxCtx->_swapchain->getCurrentImage();
		VkDependencyInfo info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
		info.imageMemoryBarrierCount = 1;
		info.pImageMemoryBarriers = &barrier;
		vkCmdPipelineBarrier2(_wrapper->_cmdBuf, &info);

		_gxCtx->_swapchain->_vkCurrentImageLayout = newLayout;
	}
	void CommandBuffer::cmdBlitToSwapchain(InternalTextureHandle source) {
		VkImageBlit2 blitRegion = { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

		blitRegion.srcOffsets[1].x = static_cast<int32_t>(_gxCtx->getTextureExtent(source).width);
		blitRegion.srcOffsets[1].y = static_cast<int32_t>(_gxCtx->getTextureExtent(source).height);
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = static_cast<int32_t>(_gxCtx->getSwapchainExtent().width);
		blitRegion.dstOffsets[1].y = static_cast<int32_t>(_gxCtx->getSwapchainExtent().height);
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo { .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
		blitInfo.srcImage = _gxCtx->getTextureImage(source);
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.dstImage = _gxCtx->_swapchain->getCurrentImage();
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2KHR(_wrapper->_cmdBuf, &blitInfo);
	}
	void CommandBuffer::cmdBindRenderPipeline(InternalPipelineHandle handle) {
		if (handle.empty()) {
			LOG_USER(LogType::Warning, "Binded render pipeline was empty/invalid!");
			return;
		}
		// use _currentPipeline
		_currentPipeline = handle;

		const RenderPipeline* pipeline = _gxCtx->resolveRenderPipeline(_currentPipeline);

		if (_lastBoundPipeline != pipeline->_vkPipeline) {
			_lastBoundPipeline = pipeline->_vkPipeline;
			vkCmdBindPipeline(_wrapper->_cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->_vkPipeline);
			_gxCtx->bindDefaultDescriptorSets(_wrapper->_cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->_vkPipelineLayout);
		}
	}
	void CommandBuffer::bufferBarrier(InternalBufferHandle bufhandle, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage) {
		AllocatedBuffer* buf = _gxCtx->_bufferPool.get(bufhandle);

		VkBufferMemoryBarrier2 barrier = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.srcStageMask = srcStage,
				.srcAccessMask = 0,
				.dstStageMask = dstStage,
				.dstAccessMask = 0,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.buffer = buf->_vkBuffer,
				.offset = 0,
				.size = VK_WHOLE_SIZE,
		};
		if (srcStage & VK_PIPELINE_STAGE_2_TRANSFER_BIT) {
			barrier.srcAccessMask |= VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
		} else {
			barrier.srcAccessMask |= VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
		}
		if (dstStage & VK_PIPELINE_STAGE_2_TRANSFER_BIT) {
			barrier.dstAccessMask |= VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
		} else {
			barrier.dstAccessMask |= VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
		}
		if (dstStage & VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT) {
			barrier.dstAccessMask |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
		}
		if (buf->_vkUsageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
			barrier.dstAccessMask |= VK_ACCESS_2_INDEX_READ_BIT;
		}
		const VkDependencyInfo depInfo = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.bufferMemoryBarrierCount = 1,
				.pBufferMemoryBarriers = &barrier,
		};
		vkCmdPipelineBarrier2(_wrapper->_cmdBuf, &depInfo);
	}
	// current issues with host buffer
	void CommandBuffer::cmdUpdateBuffer(InternalBufferHandle bufhandle, size_t offset, size_t size, const void* data) {
		ASSERT(bufhandle.valid());
		ASSERT(size && size <= 65536);
		ASSERT(size % 4 == 0);
		ASSERT(offset % 4 == 0);
		AllocatedBuffer* buf = _gxCtx->_bufferPool.get(bufhandle);
		if (!data) {
			LOG_USER(LogType::Warning, "You are updating buffer '{}' with empty data.", buf->_debugName);
			return;
		}

		bufferBarrier(bufhandle, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT);

		vkCmdUpdateBuffer(_wrapper->_cmdBuf, buf->_vkBuffer, offset, size, data);

		VkPipelineStageFlags2 dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if (buf->_vkUsageFlags & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
			dstStage |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
		}
		bufferBarrier(bufhandle, VK_PIPELINE_STAGE_2_TRANSFER_BIT, dstStage);
	}
	void CommandBuffer::cmdCopyImageToBuffer(InternalTextureHandle source, InternalBufferHandle destination, const VkBufferImageCopy& region) {
		VkImageLayout currentLayout = _gxCtx->getTextureCurrentLayout(source);
		if (currentLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL || currentLayout != VK_IMAGE_LAYOUT_GENERAL) {
			cmdTransitionLayout(source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}
		VkImageLayout properLayout = _gxCtx->getTextureCurrentLayout(source);
		vkCmdCopyImageToBuffer(_wrapper->_cmdBuf, _gxCtx->getTextureImage(source), properLayout, _gxCtx->getAllocatedBuffer(destination)->_vkBuffer, 1, &region);
	}
}



















