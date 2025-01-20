//
// Created by Hayden Rivas on 1/16/25.
//
#include "EditorGui.h"
namespace Slate {
	void EditorGui::OnViewportPanelUpdate(InputSystem& inputSystem) {
		ImGui::Begin("Viewport");
		// reuse logic from first Slate engine
		if (inputSystem.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
			// only enter camera control mode if not already active and viewport window is hovered
			if (!_isCameraControlActive && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
				&& !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()) {
				ImGui::SetWindowFocus();
				_cameraRef->isFirstMouse = true;  // reset cursor position tracking for smooth transition
				_isCameraControlActive = true;
				inputSystem.SetInputMode(GLFW_CURSOR_DISABLED);
			}
			// if already active, continue to update
			if (_isCameraControlActive) {
				glm::ivec2 pos = inputSystem.GetMousePosition();
				_cameraRef->ProcessMouse(pos.x, pos.y);
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			}
		} else if (_isCameraControlActive) {
			_isCameraControlActive = false;
			inputSystem.SetInputMode(GLFW_CURSOR_NORMAL);
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}
		ImVec2 size = ImGui::GetContentRegionAvail();
		_cameraRef->OnResize(size.x, size.y);
		ImGui::Image(sceneTexture, size);
		ImGui::End();
	}
}