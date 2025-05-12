//
// Created by Hayden Rivas on 4/27/25.
//

#pragma once

#include <cstdint>
#include <future>

#include <volk.h>

#include "Slate/Common/Invalids.h"
#include "Slate/SubmitHandle.h"

namespace Slate {
	class VulkanImmediateCommands final {
	public:
		static constexpr uint32_t kMaxCommandBuffers = 64;
		struct CommandBufferWrapper {
			VkCommandBuffer _cmdBuf = VK_NULL_HANDLE;
			VkCommandBuffer _cmdBufAllocated = VK_NULL_HANDLE;
			SubmitHandle _handle = {};
			VkFence _fence = VK_NULL_HANDLE;
			VkSemaphore _semaphore = VK_NULL_HANDLE;
			bool _isEncoding = false;
		};


		VulkanImmediateCommands(VkDevice device, uint32_t queueFamilyIndex);
		~VulkanImmediateCommands();

		// disallow copy and assignment
		VulkanImmediateCommands(const VulkanImmediateCommands&) = delete;
		VulkanImmediateCommands& operator=(const VulkanImmediateCommands&) = delete;
	public:
		void wait(SubmitHandle handle);
		void waitAll();
		bool isReady(SubmitHandle handle, bool fastCheckNoVulkan = false) const;
		VkFence getVkFence(SubmitHandle handle) const;
		const CommandBufferWrapper& acquire();
		SubmitHandle submit(const CommandBufferWrapper& wrapper);

		void waitSemaphore(VkSemaphore semaphore);
		void signalSemaphore(VkSemaphore semaphore, uint64_t signalValue);

		inline VkSemaphore acquireLastSubmitSemaphore() { return std::exchange(_lastSubmitSemaphore.semaphore, VK_NULL_HANDLE); }
		inline SubmitHandle getLastSubmitHandle() const { return _lastSubmitHandle; };
		inline SubmitHandle getNextSubmitHandle() const { return _nextSubmitHandle; };
	private:
		void _purge();
	private:
		VkDevice _vkDevice; // injected

		VkCommandPool _vkCommandPool = VK_NULL_HANDLE;

		VkSemaphoreSubmitInfo _lastSubmitSemaphore = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		};
		VkSemaphoreSubmitInfo _waitSemaphore = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		};
		VkSemaphoreSubmitInfo _signalSemaphore = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.stageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		};

		VkQueue _vkQueue = VK_NULL_HANDLE;
		uint32_t _queueFamilyIndex = Invalid<uint32_t>;

		CommandBufferWrapper _buffers[kMaxCommandBuffers];

		SubmitHandle _lastSubmitHandle = SubmitHandle();
		SubmitHandle _nextSubmitHandle = SubmitHandle();

		uint32_t _numAvailableCommandBuffers = kMaxCommandBuffers;
		uint32_t _submitCounter = 1;
	};

}