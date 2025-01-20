//
// Created by Hayden Rivas on 1/17/25.
//
#pragma once
#include <glm/detail/type_mat4x4.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace Slate {
	struct Version {
		uint8_t major = 0;
		uint8_t minor = 0;
		uint8_t patch = 0;

		// comparision operators!
		bool operator==(const Version& other) const {
			return major == other.major && minor == other.minor && patch == other.patch;
		}
		bool operator!=(const Version& other) const {
			return !(*this == other);
		}
		bool operator<(const Version& other) const {
			if (major != other.major) return major < other.major;
			if (minor != other.minor) return minor < other.minor;
			return patch < other.patch;
		}
		bool operator>(const Version& other) const {
			return other < *this;
		}
		bool operator<=(const Version& other) const {
			return !(other < *this);
		}
		bool operator>=(const Version& other) const {
			return !(*this < other);
		}
		// returns the version with . (period) seperating each value
		const char* GetVersionAsString() const {
			static char versionString[12]; // max of 11 char, including null terminator "255.255.255\0"
			int length = snprintf(versionString, sizeof(versionString), "%u.%u.%u", major, minor, patch);
			if (length < 0 || length >= static_cast<int>(sizeof(versionString))) {
				return "Error Getting Version!";
			}
			return versionString;
		};
	};
	struct VulkanInstanceInfo {
		const char* app_name = "Unnamed_App";
		Version app_version;
		const char* engine_name = "Unnamed_Engine";
		Version engine_version;
	};

	struct Vertex {
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
	};




	namespace vktypes {

		struct AllocatedImage {
			VkImage image;
			VkImageView imageView;
			VmaAllocation allocation;
			VkExtent3D imageExtent;
			VkFormat imageFormat;

			VkExtent2D GetExtent2D() {
				return VkExtent2D { imageExtent.width, imageExtent.height };
			}
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
			glm::mat4 worldMatrix;
			VkDeviceAddress vertexBuffer;
		};
	}
}