//
// Created by Hayden Rivas on 1/9/25.
//
#pragma once

// external headers
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

// Slate Headers
#include "BaseSystem.h"
#include "Ref.h"
#include "VulkanEngine.h"
#include "Window.h"

namespace Slate {
	class RenderSystem : BaseSystem {
	public:
		VulkanEngine& GetEngine() { return _vulkanEngine; };
	private:
		VulkanEngine _vulkanEngine;
	private:
		void Initialize() override;
		void Shutdown() override;
		friend class Application;
	};
}


