//
// Created by Hayden Rivas on 4/27/25.
//

#pragma once

#include <Slate/Common/Invalids.h>
#include <Slate/VK/vkimpl.h>

// forward declare
namespace vkb {
	class Instance;
	class Device;
}
class GLFWwindow;

namespace Slate {
	class VulkanInstanceInfo;

	struct DeviceQueues
	{
		VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
		uint32_t graphicsQueueFamilyIndex = Invalid<uint32_t>;
		VkQueue vkPresentQueue = VK_NULL_HANDLE;
		uint32_t presentQueueFamilyIndex = Invalid<uint32_t>;
	};

	class GXBackbone final {
	public:
		GXBackbone() = default;
		~GXBackbone() = default;
		void initialize(GLFWwindow* glfWwindow, VulkanInstanceInfo info);
		void terminate();
		// disallow copy and assignment
		GXBackbone(const GXBackbone&) = delete;
		GXBackbone& operator=(const GXBackbone&) = delete;
	public:
		// functions typically required by gx, CommandBuffer, Immediate, Swapchain or other abstractions
		inline VkInstance getInstance() const { return _vkInstance; }
		inline VkSurfaceKHR getSurface() const { return _vkSurfaceKHR; }
		inline VkPhysicalDevice getPhysicalDevice() const { return _vkPhysicalDevice; }
		inline VkDevice getDevice() const { return _vkDevice; };
		inline VmaAllocator getAllocator() const { return _vmaAllocator; }
		inline VkQueue getGraphicsQueue() const { return _queues.vkGraphicsQueue; };
		inline uint32_t getGraphicsQueueFamilyIndex() const { return _queues.graphicsQueueFamilyIndex; }
		inline VkQueue getPresentQueue() const { return _queues.vkPresentQueue; };
		inline uint32_t getPresentQueueFamilyIndex() const { return _queues.presentQueueFamilyIndex; }

		inline VkPhysicalDeviceProperties getPhysDeviceProperties() const { return _vkPhysDeviceProperties; };
#if defined(VK_API_VERSION_1_3)
		inline VkPhysicalDeviceVulkan13Properties getPhysDevicePropertiesV13() const { return _vkPhysDeviceVulkan13Properties; };
#endif
#if defined(VK_API_VERSION_1_2)
		inline VkPhysicalDeviceVulkan12Properties getPhysDevicePropertiesV12() const { return _vkPhysDeviceVulkan12Properties; };
#endif
#if defined(VK_API_VERSION_1_1)
		inline VkPhysicalDeviceVulkan11Properties getPhysDevicePropertiesV11() const { return _vkPhysDeviceVulkan11Properties; };
#endif
	private:
		VmaAllocator _vmaAllocator                 = VK_NULL_HANDLE;
		VkInstance _vkInstance                     = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT _vkDebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR _vkSurfaceKHR                 = VK_NULL_HANDLE;
		VkPhysicalDevice _vkPhysicalDevice         = VK_NULL_HANDLE;
		VkDevice _vkDevice                         = VK_NULL_HANDLE;
		// maybe move this later
		DeviceQueues _queues;

		// properties
		VkPhysicalDeviceProperties _vkPhysDeviceProperties = {};
#if defined(VK_API_VERSION_1_3)
		VkPhysicalDeviceVulkan13Properties _vkPhysDeviceVulkan13Properties = {};
#endif
#if defined(VK_API_VERSION_1_2)
		VkPhysicalDeviceVulkan12Properties _vkPhysDeviceVulkan12Properties = {};
#endif
#if defined(VK_API_VERSION_1_1)
		VkPhysicalDeviceVulkan11Properties _vkPhysDeviceVulkan11Properties = {};
#endif
	private:
		void _createInstance(vkb::Instance& vkb_instance, VulkanInstanceInfo info);
		void _createSurface(GLFWwindow* glfWwindow);
		void _createDevices(vkb::Instance& vkb_instance, vkb::Device& vkb_device);
		void _createAllocator();
		void _createQueues(vkb::Device& vkb_device);
	};
}