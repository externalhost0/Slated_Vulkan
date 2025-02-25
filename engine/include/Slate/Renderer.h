//
// Created by Hayden Rivas on 1/9/25.
//
#pragma once


// Slate Headers
#include "VulkanEngine.h"
#include "BaseSystem.h"
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


