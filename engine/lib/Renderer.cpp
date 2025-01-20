//
// Created by Hayden Rivas on 1/9/25.
//
#include <Slate/Renderer.h>
#include <Slate/Expect.h>

namespace Slate {
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	void RenderSystem::Initialize() {
		_isInitialized = true;
	}
	void RenderSystem::Shutdown() {
		EXPECT(_isInitialized, "RenderSystem has not been initialized!")
		_vulkanEngine.Destroy();
	}
}












