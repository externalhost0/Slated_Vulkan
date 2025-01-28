//
// Created by Hayden Rivas on 1/17/25.
//
#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/detail/type_mat4x4.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "vkinfo.h"
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
			VkExtent3D imageExtent;
			VkFormat imageFormat;
			VmaAllocation allocation;

			VkExtent2D GetExtent2D() { return VkExtent2D { imageExtent.width, imageExtent.height }; }

			void Destroy(VkDevice& device, VmaAllocator& allocator) const {
				vkDestroyImageView(device, imageView, nullptr);
				vmaDestroyImage(allocator, this->image, this->allocation);
			}
			void Resize(VkDevice& device, VmaAllocator& allocator, VkExtent2D extent2D) {
				Destroy(device, allocator);
				VkExtent3D tempExtent = {
						extent2D.width,
						extent2D.height,
						1
				};

				VkImageUsageFlags drawImageUsages = {};
				drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
				drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

				VkImageCreateInfo rimg_info = vkinfo::CreateImageInfo(this->imageFormat, drawImageUsages, tempExtent);

				//for the draw image, we want to allocate it from gpu local memory
				VmaAllocationCreateInfo rimg_allocinfo = {};
				rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
				rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

				//allocate and create the image
				vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &image, &allocation, nullptr);

				//build a image-view for the draw image to use for rendering
				VkImageViewCreateInfo rview_info = vkinfo::CreateImageViewInfo(this->imageFormat, this->image, VK_IMAGE_ASPECT_COLOR_BIT);
				vkCreateImageView(device, &rview_info, nullptr, &this->imageView);
			}
		};

		struct AllocatedBuffer {
			VkBuffer buffer;
			VmaAllocation allocation;
			VmaAllocationInfo info;

			void Destroy(VmaAllocator& allocator) const {
				vmaDestroyBuffer(allocator, this->buffer, this->allocation);
			}
		};

		// holds the resources needed for a mesh
		struct GPUMeshBuffers {
			AllocatedBuffer indexBuffer;
			AllocatedBuffer vertexBuffer;

			VkDeviceAddress vertexBufferAddress;

			void Destroy(VmaAllocator& allocator) const {
				indexBuffer.Destroy(allocator);
				vertexBuffer.Destroy(allocator);
			}
		};

		struct GPUUniformBufferObject {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
		};
		// push constants for our mesh object draws
		struct GPUDrawPushConstants {
			glm::mat4 renderMatrix;
			VkDeviceAddress vertexBufferAddress;
		};

		struct MeshData {
			uint32_t vertexCount;

			uint32_t indexCount;
			vktypes::AllocatedBuffer indexBuffer;

			GPUDrawPushConstants constants;

			bool IsIndexed() const { return indexCount > 0; }
		};

	}
}