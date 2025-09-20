//
// Created by Hayden Rivas on 4/27/25.
//

//#if defined(SLATE_OS_MACOS)
//#define VK_USE_PLATFORM_MACOS_MVK
//#define GLFW_EXPOSE_NATIVE_COCOA
//#elif defined(SLATE_OS_WINDOWS)
//#define VK_USE_PLATFORM_WIN32_KHR
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <windows.h>
//#elif defined(SLATE_OS_LINUX)
//#define VK_USE_PLATFORM_WAYLAND_KHR
//#define GLFW_EXPOSE_NATIVE_WAYLAND
//#endif


#include "Slate/GXBackend.h"
#include "Slate/Common/HelperMacros.h"
#include "Slate/GX.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <VkBootstrap.h>

#if defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#endif

namespace Slate {
	void GXBackend::initialize(GLFWwindow* glfWwindow, Slate::VulkanInstanceInfo info) {
		_hasSurface = (glfWwindow) ? true : false;
		static bool initz = false;
		ASSERT_MSG(!initz, "You can NEVER have more than one GXBackend instance!");
		vkb::Instance vkb_instance;
		_createInstance(vkb_instance, info);
		if (_hasSurface) _createSurface(glfWwindow);
		vkb::Device vkb_device;
		_createDevices(vkb_instance, vkb_device);
		_createAllocator();
		_createQueues(vkb_device);
		initz = true;
	}
	void GXBackend::terminate() {
		// destroy vma allocator
		// if you get validation errors regarding "device memory not destroyed" its this guy
		vmaDestroyAllocator(_vmaAllocator);
		// destroy pure vk objects
		vkDestroyDevice(_vkDevice, nullptr);
		vkDestroyDebugUtilsMessengerEXT(_vkInstance, _vkDebugMessenger, nullptr);
		if (_hasSurface) {
			vkDestroySurfaceKHR(_vkInstance, _vkSurfaceKHR, nullptr);
		}
		vkDestroyInstance(_vkInstance, nullptr);
	}

	void GXBackend::_createInstance(vkb::Instance& vkb_instance, Slate::VulkanInstanceInfo info) {
		VkResult volk_result = volkInitialize();
		ASSERT_MSG(volk_result == VK_SUCCESS, "Volk failed to initialize!");


		vkb::InstanceBuilder builder;
		auto instance_result = builder
									   .set_app_name(info.app_name)
									   .set_app_version(info.app_version.major, info.app_version.minor, info.app_version.patch)
									   .set_engine_name(info.engine_name)
									   .set_engine_version(info.engine_version.major, info.engine_version.minor, info.engine_version.patch)
#if defined(SLATE_DEBUG)
									   .request_validation_layers()
									   .use_default_debug_messenger()
#endif
									   .require_api_version(VK_API_VERSION_1_3)
									   .set_headless(!_hasSurface)
									   .build();
		ASSERT_MSG(instance_result.has_value(), "Failed to create Vulkan instance. Error: {}", instance_result.error().message().c_str());

		vkb::Instance& vkbinstance = instance_result.value();
		vkb_instance = vkbinstance;
		_vkInstance = vkbinstance.instance;
		_vkDebugMessenger = vkbinstance.debug_messenger;

		volkLoadInstance(_vkInstance);
	}
	void GXBackend::_createSurface(GLFWwindow* glfWwindow) {
		VkResult surface_result = VK_NOT_READY;
#if defined(VK_USE_PLATFORM_MACOS_MVK) && defined(GLFW_EXPOSE_NATIVE_COCOA)
		VkMacOSSurfaceCreateInfoMVK surface_ci = {};
		surface_ci.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surface_ci.pNext = nullptr;
		surface_ci.flags = 0;
		surface_ci.pView = glfwGetCocoaWindow(glfWwindow);
		surface_result = vkCreateMacOSSurfaceMVK(_vkInstance, &surface_ci, nullptr, &_vkSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WIN32_KHR) && defined(GLFW_EXPOSE_NATIVE_WIN32)
		VkWin32SurfaceCreateInfoKHR surface_ci = {};
		surface_ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_ci.pNext = nullptr;
		surface_ci.flags = 0;
		surface_ci.hwnd = glfwGetWin32Window(glfWwindow);
		surface_ci.hinstance = GetModuleHandle(nullptr);
		surface_result = vkCreateWin32SurfaceKHR(_vkInstance, &surface_ci, nullptr, &_vkSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR) && defined(GLFW_EXPOSE_NATIVE_WAYLAND)
		VkWaylandSurfaceCreateInfoKHR surface_ci = {};
		surface_ci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surface_ci.pNext = nullptr;
		surface_ci.flags = 0;
		surface_ci.surface = glfwGetWaylandWindow(glfWwindow);
		surface_ci.display = glfwGetWaylandDisplay();
		surface_result = vkCreateWaylandSurfaceKHR(_vkInstance, &surface_ci, nullptr, &_vkSurfaceKHR);
#else
		surface_result = glfwCreateWindowSurface(_vkInstance, glfWwindow, nullptr, &_vkSurfaceKHR);
		ASSERT_MSG(surface_result == VK_SUCCESS, "Failed window surface creation!");
#endif
	}
	void GXBackend::_createDevices(vkb::Instance& vkb_instance, vkb::Device& vkb_device) {
		// vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.synchronization2 = true;
		features13.dynamicRendering = true;
		features13.subgroupSizeControl = true;
		features13.maintenance4 = true;

		//vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.shaderInt8 = true;
		features12.shaderFloat16 = true;
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;
		features12.timelineSemaphore = true;
		features12.scalarBlockLayout = true;
		features12.uniformAndStorageBuffer8BitAccess = true;
		features12.uniformBufferStandardLayout = true;

		features12.shaderSampledImageArrayNonUniformIndexing = true;
		features12.shaderStorageImageArrayNonUniformIndexing = true;

		features12.descriptorBindingSampledImageUpdateAfterBind = true;
		features12.descriptorBindingStorageImageUpdateAfterBind = true;
		features12.descriptorBindingUpdateUnusedWhilePending = true;
		features12.descriptorBindingVariableDescriptorCount = true;
		features12.descriptorBindingPartiallyBound = true;

		features12.runtimeDescriptorArray = true;

		// vulkan 1.1 features
		VkPhysicalDeviceVulkan11Features features11 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		features11.storageBuffer16BitAccess = true;
		features11.shaderDrawParameters = true;

		//vulkan 1.0 features
		VkPhysicalDeviceFeatures features = {};
		features.vertexPipelineStoresAndAtomics = true;
		features.fragmentStoresAndAtomics = true;
		features.fillModeNonSolid = true;
		features.independentBlend = true;
		features.shaderInt64 = true;
		features.multiDrawIndirect = true;
		features.drawIndirectFirstInstance = true;
		features.samplerAnisotropy = true;
		features.shaderImageGatherExtended = true;
		features.samplerAnisotropy = true;

		vkb::PhysicalDeviceSelector selector{vkb_instance};
		auto physical_result = selector
									   .set_minimum_version(1, 2)
									   .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
									   .add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
									   .add_required_extension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME)

									   .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)

									   .add_required_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME)
									   .add_required_extension(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME)

									   // external memory
#if !defined(__APPLE__)
									   .add_required_extension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)
#endif


//									   .set_required_features_13(features13) //cant be on until 1.3 mvk
									   .set_required_features_12(features12)
									   .set_required_features_11(features11)
									   .set_required_features(features)
									   .set_surface(_vkSurfaceKHR)
									   .select();
		ASSERT_MSG(physical_result.has_value(), "Failed to select Vulkan Physical Device. Error: {}", physical_result.error().message().c_str());

		vkb::PhysicalDevice& vkbphysdevice = physical_result.value();
		_vkPhysicalDevice = vkbphysdevice.physical_device;
		_vkPhysDeviceProperties = vkbphysdevice.properties;

		// get properties we can query later in case we need to
		{
			VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
			VkPhysicalDeviceVulkan13Properties props13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
			VkPhysicalDeviceVulkan12Properties props12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
			VkPhysicalDeviceVulkan11Properties props11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
			props.pNext = &props13;
			props13.pNext = &props12;
			props12.pNext = &props11;
			vkGetPhysicalDeviceProperties2(_vkPhysicalDevice, &props);
			_vkPhysDeviceVulkan13Properties = props13;
			_vkPhysDeviceVulkan12Properties = props12;
			_vkPhysDeviceVulkan11Properties = props11;
		}

		// i have no idea why this is a physica device feature but im adding it into the logical device builder
		// ok so basically to not set off the validation layer, we need to use this on the physical device, we can uncomment required features 13 when 1.3 is ready for moltenvk
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		dynamic_rendering_feature.pNext = nullptr;
		dynamic_rendering_feature.dynamicRendering = VK_TRUE;

		VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR };
		synchronization2_feature.pNext = nullptr;
		synchronization2_feature.synchronization2 = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state_feature = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		dynamic_state_feature.pNext = nullptr;
		dynamic_state_feature.extendedDynamicState = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamic_state_feature_2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT };
		dynamic_state_feature_2.pNext = nullptr;
		dynamic_state_feature_2.extendedDynamicState2 = VK_TRUE;


		// GET DEVICE
		vkb::DeviceBuilder device_builder{vkbphysdevice};
		auto device_result = device_builder
									 .add_pNext(&dynamic_rendering_feature)
									 .add_pNext(&synchronization2_feature)
									 .add_pNext(&dynamic_state_feature)
									 .add_pNext(&dynamic_state_feature_2)
									 .build();
		ASSERT_MSG(device_result.has_value(), "Failed to create Vulkan device. Error: {}", device_result.error().message().c_str());

		vkb::Device& vkbdevice = device_result.value();
		vkb_device = vkbdevice;
		_vkDevice = vkbdevice.device;

		volkLoadDevice(_vkDevice);

		// alias KHR and EXT functions
#if defined(__APPLE__)
		vkCmdBeginRendering = vkCmdBeginRenderingKHR;
		vkCmdEndRendering = vkCmdEndRenderingKHR;
		vkCmdSetDepthWriteEnable = vkCmdSetDepthWriteEnableEXT;
		vkCmdSetDepthTestEnable = vkCmdSetDepthTestEnableEXT;
		vkCmdSetDepthCompareOp = vkCmdSetDepthCompareOpEXT;
		vkCmdSetDepthBiasEnable = vkCmdSetDepthBiasEnableEXT;
		vkCmdPipelineBarrier2 = vkCmdPipelineBarrier2KHR;
		vkQueueSubmit2 = vkQueueSubmit2KHR;
#endif
	}
	void GXBackend::_createAllocator() {
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = _vkPhysicalDevice;
		allocatorInfo.device = _vkDevice;
		allocatorInfo.instance = _vkInstance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
//		allocatorInfo.pTypeExternalMemoryHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		const VmaVulkanFunctions functions = {
				.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
				.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};
		allocatorInfo.pVulkanFunctions = &functions;
		VkResult allocator_result = vmaCreateAllocator(&allocatorInfo, &_vmaAllocator);
		ASSERT_MSG(allocator_result == VK_SUCCESS, "Failed to create vma Allocator!");
	}
	void GXBackend::_createQueues(vkb::Device& vkb_device) {
		// graphics queue
		auto graphics_queue_result = vkb_device.get_queue(vkb::QueueType::graphics);
		ASSERT_MSG(graphics_queue_result.has_value(), "Failed to get graphics queue. Error: {}", graphics_queue_result.error().message().c_str());
		_queues.vkGraphicsQueue = graphics_queue_result.value();

		// graphics queue index
		auto graphics_queue_index_result = vkb_device.get_queue_index(vkb::QueueType::graphics);
		ASSERT_MSG(graphics_queue_index_result.has_value(), "Failed to get graphics queue index/family. Error: {}", graphics_queue_index_result.error().message().c_str());
		_queues.graphicsQueueFamilyIndex = graphics_queue_index_result.value();

		if (!_hasSurface) return;

		// present queue
		auto present_queue_result = vkb_device.get_queue(vkb::QueueType::present);
		ASSERT_MSG(present_queue_result.has_value(), "Failed to get present queue. Error: {}", present_queue_result.error().message().c_str());
		_queues.vkPresentQueue = present_queue_result.value();

		// present queue index
		auto present_queue_index_result = vkb_device.get_queue_index(vkb::QueueType::present);
		ASSERT_MSG(present_queue_index_result.has_value(), "Failed to get present queue index/family. Error: {}", present_queue_index_result.error().message().c_str());
		_queues.presentQueueFamilyIndex = present_queue_index_result.value();
	}
}