//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

#include <glm/glm.hpp>
#include "Window.h"
#include "BaseSystem.h"

namespace Slate {
	enum struct MouseShape {
		CROSSHAIR,
		CENTER
	};
	class InputSystem {
	public:

		static bool IsKeyPressed(int key, int keystate = GLFW_PRESS);
		static bool IsMouseButtonPressed(int button);
		static std::pair<float, float> GetMousePosition();

		static void SetInputMode(int mode);
		static int GetInputMode();
		static void SetCursor(MouseShape shape);

	public:
		static void InjectWindow(GLFWwindow* window) { _pNativeWindow = window; }
	private:
		inline static GLFWwindow* _pNativeWindow = nullptr;

		inline static GLFWcursor* cross_hair_cursor = nullptr;
		inline static GLFWcursor* center_cursor = nullptr;
	private:
		static void Initialize();
		static void Shutdown();
		inline static bool _isInitialized = false;

		friend class Application;
		static GLFWcursor *GetCursorFromShape(MouseShape shape);
	};

}