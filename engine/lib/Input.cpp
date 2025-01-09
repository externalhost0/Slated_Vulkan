//
// Created by Hayden Rivas on 1/8/25.
//
#include "Slate/Application.h"
#include "Slate/Input.h"
#include <GLFW/glfw3.h>

namespace Slate {

	bool InputManager::IsKeyPressed(const int key) {
		auto state = glfwGetKey(SystemLocator::Get<WindowManager>().GetWindow()->GetNativeWindow(), static_cast<int32_t>(key));
		return state == GLFW_PRESS;
	}
	bool InputManager::IsMouseButtonPressed(const int button) {
		auto state = glfwGetMouseButton(SystemLocator::Get<WindowManager>().GetWindow()->GetNativeWindow(), static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}
	glm::vec2 InputManager::GetMousePosition() {
		double x_pos, y_pos;
		glfwGetCursorPos(SystemLocator::Get<WindowManager>().GetWindow()->GetNativeWindow(), &x_pos, &y_pos);
		return {static_cast<float>(x_pos), static_cast<float>(y_pos) };
	}

	int InputManager::GetInputMode() {
		return glfwGetInputMode(SystemLocator::Get<WindowManager>().GetWindow()->GetNativeWindow(), GLFW_CURSOR);
	}
	void InputManager::SetInputMode(int mode) {
		glfwSetInputMode(SystemLocator::Get<WindowManager>().GetWindow()->GetNativeWindow(), GLFW_CURSOR, mode);
	}

}