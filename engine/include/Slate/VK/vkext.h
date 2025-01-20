//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include <vulkan/vulkan_core.h>

// the plan is once moltenvk for 1.3 is finished this header can be removed entirely
namespace Slate::vkext {
	// commands from extensions we load globally
	inline PFN_vkCmdBeginRenderingKHR vkCmdBeginRendering;
	inline PFN_vkCmdEndRenderingKHR vkCmdEndRendering;
	inline PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2;
	inline PFN_vkQueueSubmit2KHR vkQueueSubmit2;
	inline PFN_vkCmdBlitImage2KHR vkCmdBlitImage2;

	static void LoadExtensionFunctions(VkDevice device) {
		// dynamic rendering extension
		vkCmdBeginRendering = (PFN_vkCmdBeginRenderingKHR) vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
		vkCmdEndRendering   = (PFN_vkCmdEndRenderingKHR) vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");

		// synchronization2
		vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2KHR) vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR");
		vkQueueSubmit2        = (PFN_vkQueueSubmit2KHR) vkGetDeviceProcAddr(device, "vkQueueSubmit2KHR");

		// copy_commands2
		vkCmdBlitImage2 = (PFN_vkCmdBlitImage2KHR) vkGetDeviceProcAddr(device, "vkCmdBlitImage2KHR");
	}
}