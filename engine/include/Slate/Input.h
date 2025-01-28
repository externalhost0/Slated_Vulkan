//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

#include <glm/glm.hpp>
#include "Window.h"
#include "BaseSystem.h"

namespace Slate {
	class InputSystem : BaseSystem {
	public:

		static bool IsKeyPressed(int key, int keystate = GLFW_PRESS);
		static bool IsMouseButtonPressed(int button);
		static glm::ivec2 GetMousePosition();

		static void SetInputMode(int mode);
		static int GetInputMode();

	public:
		static void InjectWindow(GLFWwindow* window) { _pNativeWindow = window; }
	private:
		inline static GLFWwindow* _pNativeWindow = nullptr;
	private:
		void Initialize() override;
		void Shutdown() override;
		friend class Application;
	};

}