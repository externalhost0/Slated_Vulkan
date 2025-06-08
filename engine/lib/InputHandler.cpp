//
// Created by Hayden Rivas on 3/19/25.
//
#include "Slate/InputHandler.h"

namespace Slate {
	static constexpr int GLFWFromInputMode(InputMode mode) {
		switch (mode) {
			case InputMode::CURSOR_NORMAL: return GLFW_CURSOR_NORMAL;
			case InputMode::CURSOR_DISABLED: return GLFW_CURSOR_DISABLED;

			default: return GLFW_CURSOR_NORMAL;
		}
	}
	static constexpr InputMode InputModeFromGLFW(int mode) {
		switch (mode) {
			case GLFW_CURSOR_NORMAL: return InputMode::CURSOR_NORMAL;
			case GLFW_CURSOR_DISABLED: return InputMode::CURSOR_DISABLED;

			default: return InputMode::CURSOR_NORMAL;
		}
	}

	bool InputHandler::IsKeyPressed(KeyCode key) {
		int state = glfwGetKey(_windowInjection.getGLFWWindow(), static_cast<int>(key));
		return state == GLFW_REPEAT;
	}
	bool InputHandler::IsKeyJustClicked(KeyCode key) {
		int state = glfwGetKey(_windowInjection.getGLFWWindow(), static_cast<int>(key));
		return state == GLFW_PRESS;
	}
	bool InputHandler::IsKeyJustReleased(KeyCode key) {
		int state = glfwGetKey(_windowInjection.getGLFWWindow(), static_cast<int>(key));
		return state == GLFW_RELEASE;
	}
	bool InputHandler::IsMouseButtonPressed(MouseButtonCode button) {
		int state = glfwGetMouseButton(_windowInjection.getGLFWWindow(), static_cast<int>(button));
		return state == GLFW_REPEAT;
	}
	bool InputHandler::IsMouseButtonJustClicked(MouseButtonCode button) {
		int state = glfwGetMouseButton(_windowInjection.getGLFWWindow(), static_cast<int>(button));
		return state == GLFW_PRESS;
	}
	bool InputHandler::IsMouseButtonJustReleased(MouseButtonCode button) {
		int state = glfwGetMouseButton(_windowInjection.getGLFWWindow(), static_cast<int>(button));
		return state == GLFW_RELEASE;
	}
	std::pair<float, float> InputHandler::GetMousePosition() const {
		double xpos, ypos;
		glfwGetCursorPos(_windowInjection.getGLFWWindow(), &xpos, &ypos);
		return { xpos, ypos };
	}
	std::pair<float, float> InputHandler::GetMouseDeltaPosition() const {
		double xpos, ypos;
		glfwGetCursorPos(_windowInjection.getGLFWWindow(), &xpos, &ypos);
		return { _lastFrameMousePos.first - xpos, _lastFrameMousePos.second - ypos };
	}

	void InputHandler::SetInputMode(InputMode new_mode) {
		glfwSetInputMode(_windowInjection.getGLFWWindow(), GLFW_CURSOR, GLFWFromInputMode(new_mode));
	}
	InputMode InputHandler::GetInputMode() const {
		auto mode = glfwGetInputMode(_windowInjection.getGLFWWindow(), GLFW_CURSOR);
		return InputModeFromGLFW(mode);
	}
}