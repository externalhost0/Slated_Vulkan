//
// Created by Hayden Rivas on 1/17/25.
//
#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <vk_mem_alloc.h>

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
		float uv_x;
		alignas(16) glm::vec3 normal;
		float uv_y;

		Vertex_Standard() = default;
		Vertex_Standard(glm::vec3 pos) : position(pos) {};
		Vertex_Standard(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) : position(pos), normal(norm), uv_x(uv.x), uv_y(uv.y) {};
	};


	enum class MaterialPassType : uint8_t {
		Opaque,
		Transparent
	};



	namespace GPU {
		// push constants for our mesh object draws
		struct DrawPushConstants {
			alignas(16) glm::mat4 modelMatrix;
			alignas(4) uint32_t id;
			alignas(8) VkDeviceAddress vertexBufferAddress;
		};

		struct CameraUBO {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			alignas(16) glm::vec3 position;
		};
		struct ExtraUBO {
			glm::vec4 color;
		};

		// some of the point options that match their shader counterpart
		struct AmbientLight {
			alignas(16) glm::vec3 Color = { 1.f, 1.f, 1.f };
			float Intensity = 0.1f;
		};
		struct DirectionalLight {
			alignas(16) glm::vec3 Color = { 1.f, 1.f, 1.f };
			float Intensity = 1.f;
			alignas(16) glm::vec3 Direction = { 0.f, -1.f, 0.f };
		};
		struct PointLight {
			alignas(16) glm::vec3 Color = { 1.f, 1.f, 1.f };
			float Intensity = 1.f;
			alignas(16) glm::vec3 Position = { 0.f, 0.f, 0.f };
			float Range = 5.f;
		};
		struct SpotLight {
			alignas(16) glm::vec3 Color = { 1.f, 1.f, 1.f };
			alignas(16) glm::vec3 Position = { 0.f, 0.f, 0.f };
			alignas(16) glm::vec3 Direction = { 0.f, -1.f, 0.f };
			alignas(4) float Intensity = 1.f;
			alignas(4) float Size = 45.f;
			alignas(4) float Blend = 1.f;
		};

		struct LightingUBO {
			AmbientLight ambient {};
			DirectionalLight directional {};
			PointLight points[4] {};

			void ClearDynamics() {
				directional.Intensity = 0.f;
				for (auto& point : points) {
					point.Intensity = 0.f;
				}

			}
		};
	}

	namespace vktypes {
		struct AllocatedImage {
			VkImage image;
			VkImageView imageView;
			VkExtent3D imageExtent;
			VkFormat imageFormat;

			VmaAllocation allocation;
			VmaAllocationInfo allocationInfo;


			VkExtent2D GetExtent2D() { return VkExtent2D{ imageExtent.width, imageExtent.height }; }
		};
		struct AllocatedBuffer {
			VkBuffer buffer;

			VmaAllocation allocation;
			VmaAllocationInfo info;
		};

		// holds the resources needed for a mesh
		struct StagingMeshBuffers {
			AllocatedBuffer indexBuffer;
			AllocatedBuffer vertexBuffer;

			VkDeviceAddress vertexBufferAddress;
		};

		struct MeshData {
			uint32_t vertexCount;

			uint32_t indexCount;
			vktypes::AllocatedBuffer indexBuffer;

			GPU::DrawPushConstants constants;

			bool IsIndexed() const { return indexCount > 0; }
		};
	}

}