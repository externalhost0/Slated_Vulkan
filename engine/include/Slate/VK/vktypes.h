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
#include "Slate/SmartPointers.h"

namespace Slate {
	class RenderEngine;

	struct VulkanInstanceInfo {
		const char* app_name = "Unnamed_App";
		Version app_version;
		const char* engine_name = "Unnamed_Engine";
		Version engine_version;
	};


	struct Vertex {
		alignas(16) glm::vec3 position;
		float uv_x;
		alignas(16) glm::vec3 normal;
		float uv_y;
		alignas(16) glm::vec4 tangent;

		Vertex() = default;
		Vertex(glm::vec3 pos) : position(pos) {};
		Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) : position(pos), normal(norm), uv_x(uv.x), uv_y(uv.y) {};
	};

	namespace GPU {
		// push constants for our mesh object draws
		struct DrawPushConstants {
			alignas(16) glm::mat4 modelMatrix = glm::mat4(1);
			alignas(8) VkDeviceAddress vertexBufferAddress = 0;
			alignas(4) uint32_t id = 0;
		};
		struct DrawPushConstantsEditorEXT {
			alignas(16) glm::mat4 modelMatrix = glm::mat4(1);
			glm::vec3 color = {0, 0, 0};
			alignas(4) uint32_t id = 0;
			alignas(8) VkDeviceAddress vertexBufferAddress = 0;
		};

		struct CameraUBO {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			alignas(16) glm::vec3 position;
		};
		// a dynamic uniform buffer
		struct ExtraUBO {
			alignas(16) glm::vec4 color;
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
			float Intensity = 1.f;
			alignas(16) glm::vec3 Position = { 0.f, 0.f, 0.f };
			float Size = 45.f;
			alignas(16) glm::vec3 Direction = { 0.f, -1.f, 0.f };
			float Blend = 1.f;
		};

		constexpr uint MAX_POINT_LIGHTS = 4;
		constexpr uint MAX_SPOT_LIGHTS = 4;

		struct LightingUBO {
			AmbientLight ambient {};
			DirectionalLight directional {};
			PointLight points[MAX_POINT_LIGHTS] {};
			SpotLight spots[MAX_SPOT_LIGHTS] {};

			void ClearDynamics() {
				directional.Intensity = 0.f;
				for (auto& point : points) {
					point.Intensity = 0.f;
				}
				for (auto& spot : spots) {
					spot.Intensity = 0.f;
				}
			}

		};
	}

	enum class MaterialPassType : uint8_t {
		Opaque,
		Transparent
	};

	namespace vktypes {
		struct AllocatedImage {
		public:
			VkImage getImage() const { return this->image; }
			VkImageView getImageView() const { return this->imageView; }
			VkFormat getFormat() const { return this->imageFormat; }
			VkExtent2D getExtent2D() const { return VkExtent2D{ imageExtent.width, imageExtent.height }; }
		private:
			VkImage image = VK_NULL_HANDLE;
			VkImageView imageView = VK_NULL_HANDLE;
			VkExtent3D imageExtent = {};
			VkFormat imageFormat = VK_FORMAT_UNDEFINED;

			VmaAllocation allocation = nullptr;
			VmaAllocationInfo allocationInfo = {};
			friend class Slate::RenderEngine;
		};


		struct AllocatedBuffer {
		public:
			VkBuffer getBuffer() const { return this->buffer; }
			VkDeviceSize getBufferSize() const { return this->allocationInfo.size; }
			void* getMappedMemory() const { return this->allocationInfo.pMappedData; }

			void writeToBuffer(VmaAllocator allocator, void* data, VkDeviceSize size, VkDeviceSize offset) const {
				vmaCopyMemoryToAllocation(allocator, data, this->allocation, offset, size);
			};
			VkDeviceSize getAlignmentSize() const {
				if (this->minOffsetAlignment > 0) {
					return (this->allocationInfo.size + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
				}
				return this->allocationInfo.size;
			}
		private:
			VkBuffer buffer = VK_NULL_HANDLE;

			VmaAllocation allocation = nullptr;
			VmaAllocationInfo allocationInfo = {};
			VkDeviceSize minOffsetAlignment = 1; // set in buffer creation

			friend class Slate::RenderEngine; // for creation
		};

		// holds the resources needed for a mesh
		struct StagingMeshBuffers {
			AllocatedBuffer indexBuffer;
			AllocatedBuffer vertexBuffer;

			VkDeviceAddress vertexBufferAddress = {};
		};
	}

//	class ShaderResource {
//	public:
//		ShaderResource() = default;
//		~ShaderResource() = default;
//
//		std::string GetFilename() const { return this->filename; }
//
//	private:
//		VkShaderModule module = VK_NULL_HANDLE;
//		std::string filename = "NULL";
//
//		friend class VulkanEngine;
//	};
}