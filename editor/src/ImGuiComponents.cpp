//
// Created by Hayden Rivas on 2/4/25.
//

#include <IconFontCppHeaders/IconsLucide.h>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <fmt/base.h>
#include <nfd.h>
#include <cmath>

#include "ImGuiComponents.h"
#include "Utilities.h"

namespace SlateGui {


	// TODO: the entire slategui needs to be properly written
	bool ToggleButton(const char* label) {
		static bool state = false;
		if (ImGui::Button(label)) {
			state = !state;
		}
		return state;
	}

	bool ColorField(glm::vec3& color, const char* label, SlateGuiColorEditFlags_ flags) {
		static bool toggle;
		bool special = false;


		ImVec2 size = { ImGui::GetFontSize()*6, 0 };
		if (ImGui::ColorButton("##button", ImVec4(color.r, color.g, color.b, 1.0), ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip, size))
			ImGui::OpenPopup("hi-picker");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay)) {
			ImGui::BeginTooltip();
			// TODO add tooltip content

			ImGui::EndTooltip();
		}

		if (toggle && ImGui::IsItemHovered()) {
			// fun little easter egg if you try to sample the colorbutton above
			static float time = 0;
			float r = 0.5f + 0.5f * sin(time);
			float g = 0.5f + 0.5f * sin(time + 2.0f);
			float b = 0.5f + 0.5f * sin(time + 4.0f);
			time += 0.2f;
			color = { r, g, b };
			special = true;
		}
		ImGui::SameLine();

		ImVec4 onColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
		ImVec4 offColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, toggle ? onColor : offColor);
		if (ImGui::Button(ICON_LC_PIPETTE)) {
			toggle = !toggle;
		}
		ImGui::PopStyleColor();
		if (toggle && !special) {
			std::array<int, 4> result = Slate::Utilities::ColorPickScreenFunc();
			ImVec4 convert = ImColor(result[0], result[1], result[2]);
			color = { convert.x, convert.y, convert.z };
			fmt::print("Setting Color DEBUG\n");
		}
		if (toggle) {
			// FIXME: for some reasonwe have to repeatedly set this, the cursor flickers, reseting back to the arrow!!
//			Slate::InputSystem::SetCursor(Slate::MouseShape::CROSSHAIR);
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			toggle = false;
		}



		if (ImGui::BeginPopup("hi-picker", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
			if (ImGui::IsKeyPressed(ImGuiKey_Enter)) ImGui::CloseCurrentPopup();
			ImGui::ColorPicker3("##picker", const_cast<float*>(&color[0]));
			ImGui::EndPopup();
			return true;
		}
		return false;
	}


	bool ColorField(glm::vec4& color, const char* label, SlateGuiColorEditFlags_ flags) {
		if (ImGui::ColorButton("##button", ImVec4(color.r, color.g, color.b, color.a)))
			ImGui::OpenPopup("hi-picker");

		if (ImGui::BeginPopup("hi-picker", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
			if (ImGui::IsKeyPressed(ImGuiKey_Enter)) ImGui::CloseCurrentPopup();
			ImGui::ColorPicker4("##picker", const_cast<float*>(&color[0]));
			ImGui::EndPopup();
			return true;
		}
		return false;
	}
}