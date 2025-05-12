//
// Created by Hayden Rivas on 1/17/25.
//
#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <vk_mem_alloc.h>

#include "Slate/SmartPointers.h"
#include "Slate/Version.h"
#include "vkinfo.h"
#include "Slate/VkObjects.h"

namespace Slate {
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

		struct CameraData {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			alignas(16) glm::vec3 position;
		};
		struct DrawPushConstantsEditorEXT {
			CameraData camera;
			alignas(16) glm::mat4 modelMatrix = glm::mat4(1);
			alignas(16) glm::vec3 color = {0, 0, 0};
			alignas(4) uint32_t id = 0;
			alignas(8) VkDeviceAddress vertexBufferAddress = 0;
		};

		struct DrawPushConstants2EditorEXT {
			CameraData camera;
			alignas(16) glm::mat4 modelMatrix = glm::mat4(1);
			alignas(4) uint32_t id = 0;
			alignas(8) VkDeviceAddress vertexBufferAddress = 0;
		};

;

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

		constexpr uint8_t MAX_POINT_LIGHTS = 4;
		constexpr uint8_t MAX_SPOT_LIGHTS = 4;

		struct LightingData {
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
		struct PushConstants_EditorPrimitives {
			alignas(16) glm::mat4 modelMatrix;
			alignas(8) VkDeviceAddress vertexBufferAddress;
		};
		struct PushConstants_EditorImagesREPLACE {
			alignas(16) glm::mat4 modelMatrix;
			alignas(16) glm::vec3 color;
			alignas(8) VkDeviceAddress vertexBufferAddress;
			uint32_t id;
			uint textureId;
		};

		struct PushConstants_EditorImagesOD2 {
			glm::mat4 modelMatrix;
			glm::vec3 color;
			VkDeviceAddress vertexBufferAddress;
			uint32_t id;
			uint textureId;
		};

		struct PushConstants_EditorImages {
			glm::mat4 modelMatrix;
			glm::vec3 color;
			VkDeviceAddress vertexBufferAddress;
			uint32_t id;
			uint textureId;
		};

		// standard structs
		struct PerFrameData {
			CameraData camera;
			LightingData lighting;
			float time;
		};
		struct PerObjectData {
			alignas(16) glm::mat4 modelMatrix;
			alignas(8) VkDeviceAddress vertexBufferAddress;
			alignas(4) uint32_t id;
		};
	}
	enum class MaterialPassType : uint8_t {
		Opaque,
		Transparent
	};
}