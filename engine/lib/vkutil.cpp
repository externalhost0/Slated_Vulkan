//
// Created by Hayden Rivas on 1/16/25.
//


#include <volk.h>
#include <fmt/core.h>

#include "Slate/VK/vkutil.h"

namespace Slate::vkutil {
	struct TextureFormatProperties {
		const VkFormat format = VK_FORMAT_UNDEFINED;
		const uint8_t bytesPerBlock : 5 = 1;
		const uint8_t blockWidth : 3 = 1;
		const uint8_t blockHeight : 3 = 1;
		const uint8_t minBlocksX : 2 = 1;
		const uint8_t minBlocksY : 2 = 1;
		const bool depth : 1 = false;
		const bool stencil : 1 = false;
		const bool compressed : 1 = false;
		const uint8_t numPlanes : 2 = 1;
	};
	#define PROPS(fmt, bpb, ...) TextureFormatProperties { .format = VK_FORMAT_##fmt, .bytesPerBlock = bpb, ##__VA_ARGS__ }

	static constexpr TextureFormatProperties properties[] = {
		PROPS(UNDEFINED, 1),
		PROPS(R8_UNORM, 1),                         // R_UN8
		PROPS(R16_UINT, 2),                         // R_UI16
		PROPS(R32_UINT, 4),                         // R_UI32
		PROPS(R16_UNORM, 2),                        // R_UN16
		PROPS(R16_SFLOAT, 2),                       // R_F16
		PROPS(R32_SFLOAT, 4),                       // R_F32
		PROPS(R8G8_UNORM, 2),                       // RG_UN8
		PROPS(R16G16_UINT, 4),                      // RG_UI16
		PROPS(R32G32_UINT, 8),                      // RG_UI32
		PROPS(R16G16_UNORM, 4),                     // RG_UN16
		PROPS(R16G16_SFLOAT, 4),                    // RG_F16
		PROPS(R32G32_SFLOAT, 8),                    // RG_F32
		PROPS(R8G8B8A8_UNORM, 4),                   // RGBA_UN8
		PROPS(R32G32B32A32_UINT, 16),               // RGBA_UI32
		PROPS(R16G16B16A16_SFLOAT, 8),              // RGBA_F16
		PROPS(R32G32B32A32_SFLOAT, 16),             // RGBA_F32
		PROPS(R8G8B8A8_SRGB, 4),                    // RGBA_SRGB8
		PROPS(B8G8R8A8_UNORM, 4),                   // BGRA_UN8
		PROPS(B8G8R8A8_SRGB, 4),                    // BGRA_SRGB8
		PROPS(A2B10G10R10_UNORM_PACK32, 4),         // A2B10G10R10_UN
		PROPS(A2R10G10B10_UNORM_PACK32, 4),         // A2R10G10B10_UN
		PROPS(ETC2_R8G8B8_UNORM_BLOCK, 8, .blockWidth = 4, .blockHeight = 4, .compressed = true),   // ETC2_RGB8
		PROPS(ETC2_R8G8B8_SRGB_BLOCK, 8, .blockWidth = 4, .blockHeight = 4, .compressed = true),    // ETC2_SRGB8
		PROPS(BC7_UNORM_BLOCK, 16, .blockWidth = 4, .blockHeight = 4, .compressed = true),          // BC7_RGBA
		PROPS(D16_UNORM, 2, .depth = true),                    // Z_UN16
		PROPS(X8_D24_UNORM_PACK32, 3, .depth = true),          // Z_UN24
		PROPS(D32_SFLOAT, 4, .depth = true),                   // Z_F32
		PROPS(D24_UNORM_S8_UINT, 4, .depth = true, .stencil = true),     // Z_UN24_S_UI8
		PROPS(D32_SFLOAT_S8_UINT, 5, .depth = true, .stencil = true),    // Z_F32_S_UI8
		PROPS(G8_B8R8_2PLANE_420_UNORM, 24, .blockWidth = 4, .blockHeight = 4, .compressed = true, .numPlanes = 2), // YUV_NV12
		PROPS(G8_B8_R8_3PLANE_420_UNORM, 24, .blockWidth = 4, .blockHeight = 4, .compressed = true, .numPlanes = 3), // YUV_420p
	};
	uint32_t GetTextureBytesPerLayer(uint32_t width, uint32_t height, VkFormat format, uint32_t level) {
		const uint32_t levelWidth = std::max(width >> level, 1u);
		const uint32_t levelHeight = std::max(height >> level, 1u);

		const TextureFormatProperties props = properties[format];
		if (!props.compressed) {
			return props.bytesPerBlock * levelWidth * levelHeight;
		}
		const uint32_t blockWidth = std::max((uint32_t)props.blockWidth, 1u);
		const uint32_t blockHeight = std::max((uint32_t)props.blockHeight, 1u);
		const uint32_t widthInBlocks = (levelWidth + props.blockWidth - 1) / props.blockWidth;
		const uint32_t heightInBlocks = (levelHeight + props.blockHeight - 1) / props.blockHeight;
		return widthInBlocks * heightInBlocks * props.bytesPerBlock;
	}
	uint32_t GetTextureBytesPerPlane(uint32_t width, uint32_t height, VkFormat format, uint32_t plane) {
		const TextureFormatProperties props = properties[format];
		ASSERT(plane < props.numPlanes);
		switch (format) {
			case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
				return width * height / (plane + 1);
			case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
				return width * height / (plane ? 4 : 1);
			default:;
		}
		return GetTextureBytesPerLayer(width, height, format, 0);
	}
	VkExtent2D GetImagePlaneExtent(VkExtent2D plane0, VkFormat format, uint32_t plane) {
		switch (format) {
			case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
				return VkExtent2D{
						.width = plane0.width >> plane,
						.height = plane0.height >> plane,
				};
			case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
				return VkExtent2D{
						.width = plane0.width >> (plane ? 1 : 0),
						.height = plane0.height >> (plane ? 1 : 0),
				};
			default:;
		}
		return plane0;
	}
	uint32_t GetAlignedSize(uint32_t value, uint32_t alignment) {
		return (value + alignment - 1) & ~(alignment - 1);
	}

	VkImageAspectFlags AspectMaskFromAttachmentLayout(VkImageLayout layout) {
		switch (layout) {
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_STENCIL_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			default: return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format) {
		switch (format) {
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
				return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			default: return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	bool IsFormatDepthOrStencil(VkFormat format) {
		switch (format) {
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return true;
			default: return false;
		}
	}

	uint32_t GetBytesPerPixel(VkFormat format) {
		switch (format) {
			case VK_FORMAT_R8_UNORM:
				return 1;
			case VK_FORMAT_R16_SFLOAT:
				return 2;
			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_B8G8R8_UNORM:
				return 3;
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_R16G16_SFLOAT:
			case VK_FORMAT_R32_SFLOAT:
			case VK_FORMAT_R32_UINT:
				return 4;
			case VK_FORMAT_R16G16B16_SFLOAT:
				return 6;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32_SFLOAT:
			case VK_FORMAT_R32G32_UINT:
				return 8;
			case VK_FORMAT_R32G32B32_SFLOAT:
				return 12;
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return 16;
			default:;
		}
		ASSERT_MSG(false, "VkFormat value not handled: {}", (int)format);
		return 1;
	}
	uint32_t GetNumImagePlanes(VkFormat format) {
		switch (format) {
			case VK_FORMAT_UNDEFINED:
				return 0;
			case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
			case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
			case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
			case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
			case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
			case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
				return 3;
			case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
			case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
			case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
			case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
			case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
			case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
			case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
			case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
				return 2;
			default:
				return 1;
		}
	}

	void ImageMemoryBarrier2(VkCommandBuffer cmd, VkImage image, StageAccess src, StageAccess dst, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange range) {
		const VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,

			.srcStageMask = src.stage,
			.srcAccessMask = src.access,
			.dstStageMask = dst.stage,
			.dstAccessMask = dst.access,

			.oldLayout = oldLayout,
			.newLayout = newLayout,

			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

			.image = image,
			.subresourceRange = range
		};
		const VkDependencyInfo di = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &barrier
		};
		vkCmdPipelineBarrier2KHR(cmd, &di);
	}
	StageAccess getPipelineStageAccess(VkImageLayout layout) {
		switch (layout) {
			case VK_IMAGE_LAYOUT_UNDEFINED:
				return {
						.stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
						.access = VK_ACCESS_2_NONE,
				};
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return {
						.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
						.access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				};
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return {
						.stage = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
						.access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				};
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return {
						.stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT |
								 VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT,
						.access = VK_ACCESS_2_SHADER_READ_BIT,
				};
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return {
						.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
						.access = VK_ACCESS_2_TRANSFER_READ_BIT,
				};
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return {
						.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
						.access = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				};
			case VK_IMAGE_LAYOUT_GENERAL:
				return {
						.stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
						.access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
				};
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				return {
						.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
						.access = VK_ACCESS_2_NONE | VK_ACCESS_2_SHADER_WRITE_BIT,
				};
			default:
				ASSERT_MSG(false, "Unsupported image layout transition!");
				return {
						.stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
						.access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
				};
		}
	};


}
