//
// Created by Hayden Rivas on 1/8/25.
//
#include <GLFW/glfw3.h>
#include "Slate/Application.h"
#include "Slate/Input.h"
#include "Slate/Window.h"
#include "Slate/Debug.h"

namespace Slate {
	void InputSystem::Initialize() {
		_isInitialized = true;

		cross_hair_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		center_cursor = glfwCreateStandardCursor(GLFW_CENTER_CURSOR);
	}
	void InputSystem::Shutdown() {
		EXPECT(_isInitialized, "InputSystem has not been initialized!")

		glfwDestroyCursor(cross_hair_cursor);
		glfwDestroyCursor(center_cursor);
	}

	bool InputSystem::IsKeyPressed(int key, int keystate) {
		int state = glfwGetKey(_pNativeWindow, static_cast<int32_t>(key));
		return state == keystate;
	}
	bool InputSystem::IsMouseButtonPressed(const int button) {
		auto state = glfwGetMouseButton(_pNativeWindow, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}
	std::pair<float, float> InputSystem::GetMousePosition() {
		double x_pos, y_pos;
		glfwGetCursorPos(_pNativeWindow, &x_pos, &y_pos);
		return { x_pos, y_pos };
	}
	int InputSystem::GetInputMode() {
		return glfwGetInputMode(_pNativeWindow, GLFW_CURSOR);
	}
	void InputSystem::SetInputMode(int mode) {
		glfwSetInputMode(_pNativeWindow, GLFW_CURSOR, mode);
	}
	void InputSystem::SetCursor(MouseShape shape) {
		GLFWcursor* cursor = GetCursorFromShape(shape);
		if (!cursor) {
			throw std::runtime_error("Failed to get cursor shape!");
		}
		glfwSetCursor(_pNativeWindow, cursor);
	}

	GLFWcursor* InputSystem::GetCursorFromShape(MouseShape shape) {
		switch (shape) {
			case MouseShape::CENTER: return center_cursor;
			case MouseShape::CROSSHAIR: return cross_hair_cursor;
			default: return nullptr;
		}
	}
}