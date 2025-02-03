//
// Created by Hayden Rivas on 1/17/25.
//
#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/detail/type_mat4x4.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "Slate/GeneralTypes.h"
#include "vkinfo.h"

namespace Slate {
	struct VulkanInstanceInfo {
		const char* app_name = "Unnamed_App";
		Version app_version;
		const char* engine_name = "Unnamed_Engine";
		Version engine_version;
	};


	struct Vertex_Standard {
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 normal;
		alignas(8) glm::vec2 uv;
	};


	enum class MaterialPassType : uint8_t {
		Opaque,
		Transparent
	};




	namespace vktypes {
		struct AllocatedImage {
			VkImage image;
			VkImageView imageView;
			VkExtent3D imageExtent;
			VkFormat imageFormat;

			VmaAllocation allocation;
			VmaAllocationInfo allocationInfo;


			VkExtent2D GetExtent2D() { return VkExtent2D { imageExtent.width, imageExtent.height }; }
		};

		struct AllocatedBuffer {
			VkBuffer buffer;

			VmaAllocation allocation;
			VmaAllocationInfo info;
		};

		// holds the resources needed for a mesh
		struct GPUMeshBuffers {
			AllocatedBuffer indexBuffer;
			AllocatedBuffer vertexBuffer;

			VkDeviceAddress vertexBufferAddress;
		};

		// push constants for our mesh object draws
		struct GPUDrawPushConstants {
			glm::mat4 modelMatrix;
			uint32_t id;
			VkDeviceAddress vertexBufferAddress;
		};

		struct GPUSceneData {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
		};

		struct MeshData {
			uint32_t vertexCount;

			uint32_t indexCount;
			vktypes::AllocatedBuffer indexBuffer;

			GPUDrawPushConstants constants;

			bool IsIndexed() const { return indexCount > 0; }
		};

		struct PipelineObject {
			VkPipeline pipeline     = VK_NULL_HANDLE;
			VkPipelineLayout layout = VK_NULL_HANDLE;
		};
		struct DescriptorSetObject {
			VkDescriptorSet set;
			VkDescriptorSetLayout layout;
		};
	}
}