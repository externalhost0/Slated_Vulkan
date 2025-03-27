//
// Created by Hayden Rivas on 3/19/25.
//
#include "Slate/Systems/InputSystem.h"
#include "Slate/Systems/WindowSystem.h"
#include "Slate/SystemManager.h"

namespace Slate {
	int GLFWFromInputMode(InputMode mode) {
		switch (mode) {
			case InputMode::CURSOR_NORMAL: return GLFW_CURSOR_NORMAL;
			case InputMode::CURSOR_DISABLED: return GLFW_CURSOR_DISABLED;

			default: return GLFW_CURSOR_NORMAL;
		}
	}
	InputMode InputModeFromGLFW(int mode) {
		switch (mode) {
			case GLFW_CURSOR_NORMAL: return InputMode::CURSOR_NORMAL;
			case GLFW_CURSOR_DISABLED: return InputMode::CURSOR_DISABLED;

			default: return InputMode::CURSOR_NORMAL;
		}
	}


	void InputSystem::StartupImpl() {

	}
	void InputSystem::ShutdownImpl() {

	}
	bool InputSystem::IsKeyPressed(int key, int keystate) const {
		int state = glfwGetKey(this->owner->GetSystem<WindowSystem>()->GetCurrentWindow()->GetGlfwWindow(), static_cast<int32_t>(key));
		return state == keystate;
	}
	bool InputSystem::IsMouseButtonPressed(int button) const {
		auto state = glfwGetMouseButton(this->owner->GetSystem<WindowSystem>()->GetCurrentWindow()->GetGlfwWindow(), static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}
	std::pair<float, float> InputSystem::GetMousePosition() const {
		double xpos, ypos;
		glfwGetCursorPos(nullptr, &xpos, &ypos);
		return { xpos, ypos };
	}
	void InputSystem::SetInputMode(InputMode new_mode) {
		glfwSetInputMode(this->owner->GetSystem<WindowSystem>()->GetCurrentWindow()->GetGlfwWindow(), GLFW_CURSOR, GLFWFromInputMode(new_mode));
	}
	InputMode InputSystem::GetInputMode() const {
		auto mode = glfwGetInputMode(nullptr, GLFW_CURSOR);
		return InputModeFromGLFW(mode);
	}


}