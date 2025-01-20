//
// Created by Hayden Rivas on 1/16/25.
//
#include "EditorGui.h"
namespace Slate {
	void EditorGui::OnPropertiesPanelUpdate() {
		ImGui::Begin("Properties");

		static float axis[3];
		ImGui::SliderFloat3("transform", axis, -10.f, 10.f);
//		engine.transformtest = glm::vec3(axis[0], axis[1], axis[2]);

		ImGui::End();

	}
}