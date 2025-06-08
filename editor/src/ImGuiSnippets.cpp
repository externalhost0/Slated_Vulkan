//
// Created by Hayden Rivas on 1/22/25.
//
#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include "ImGuiSnippets.h"
namespace Slate {
	ImVec4 Adjust_Brightness(const ImVec4& color, float amount) {
		// Clamp each channel after applying the adjustment (positive or negative)
		auto saturate = [](float v) {
			return std::clamp(v, 0.0f, 1.0f);
		};
		return {
				saturate(color.x + amount),
				saturate(color.y + amount),
				saturate(color.z + amount),
				color.w // Preserve alpha
		};
	}

	ImVec4 Adjust_Saturation(const ImVec4& color, float amount) {
		// percieved luminance
		float grayscale = color.x * 0.299f + color.y * 0.587f + color.z * 0.114f;
		// lerp between original and graysscale
		float r = color.x + amount * (grayscale - color.x);
		float g = color.y + amount * (grayscale - color.y);
		float b = color.z + amount * (grayscale - color.z);
		// return the new ImVec4 with the adjusted RGB and original alpha
		return {r, g, b, color.w};
	}
	ImVec4 Adjust_HueShift(const ImVec4& color, float amount) {
		// normalize input amount
		amount = std::clamp(amount, -1.0f, 1.0f);

		// rgb to hsv for manipulation
		float r = color.x, g = color.y, b = color.z;
		float max = std::max(r, std::max(g, b)), min = std::min(r, std::min(g, b));
		float h = 0.0f, s = (max == 0.0f) ? 0.0f : (max - min) / max, v = max;

		if (max != min) {
			if (r == max) h = (g - b) / (max - min);
			else if (g == max) h = (b - r) / (max - min) + 2.0f;
			else h = (r - g) / (max - min) + 4.0f;
			h *= 60.0f;
			if (h < 0.0f) h += 360.0f;
		}

		h = fmod(h + amount * 360.0f, 360.0f);
		if (h < 0.0f) h += 360.0f;

		// convert back to RGB
		int i = int(h / 60.0f);
		float f = h / 60.0f - i, p = v * (1.0f - s), q = v * (1.0f - f * s), t = v * (1.0f - (1.0f - f) * s);
		float rgb[3] = {p, q, t};
		if (i == 0) { rgb[0] = v; rgb[1] = t; rgb[2] = p; }
		else if (i == 1) { rgb[0] = q; rgb[1] = v; rgb[2] = p; }
		else if (i == 2) { rgb[0] = p; rgb[1] = v; rgb[2] = t; }
		else if (i == 3) { rgb[0] = p; rgb[1] = q; rgb[2] = v; }
		else if (i == 4) { rgb[0] = t; rgb[1] = p; rgb[2] = v; }
		else { rgb[0] = v; rgb[1] = p; rgb[2] = q; }

		// return new, same alpha
		return {rgb[0], rgb[1], rgb[2], color.w};
	}

	void Vector3Drag(const char *label, glm::vec3* value, const char* format, float resetValue, float dragSpeed) {
		float min = 0.f;
		float max = 0.f;

		float buttonwidth = 5.f;
		float valuespacing = 10.f;

		ImGuizmo::Style guizmostyle  = ImGuizmo::GetStyle();
		ImVec4* guizmocolors = guizmostyle.Colors;

		ImGui::PushID(label);
		{
			ImGui::BeginTable("##table", 2, ImGuiTableFlags_SizingStretchProp);

			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 75.f);
			ImGui::TableSetupColumn("Values", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label);
			ImGui::TableNextColumn();
			{
				ImGui::BeginTable("##inner_table", 3, ImGuiTableFlags_SizingStretchSame);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{1, 0}); // spacing between the X, Y, Z and the actual values

				ImGui::TableNextColumn();
				{
					ImGui::PushStyleColor(ImGuiCol_Button, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_X], -0.15f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_X], 0.25f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, guizmocolors[ImGuizmo::DIRECTION_X]);
				}
				ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, buttonwidth);
				if (ImGui::Button("X")) value->x = resetValue;
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);
				ImGui::SameLine();
				ImGui::DragFloat("##X", &value->x, dragSpeed, min, max, format);

				ImGui::TableNextColumn();
				{
					ImGui::PushStyleColor(ImGuiCol_Button, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Y], -0.15f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Y], 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, guizmocolors[ImGuizmo::DIRECTION_Y]);
				}
				ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, buttonwidth);
				if (ImGui::Button("Y")) value->y = resetValue;
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);
				ImGui::SameLine();
				ImGui::DragFloat("##Y", &value->y, dragSpeed, min, max, format);

				ImGui::TableNextColumn();
				{
					ImGui::PushStyleColor(ImGuiCol_Button, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Z], -0.15f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Z], 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, guizmocolors[ImGuizmo::DIRECTION_Z]);
				}
				ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, buttonwidth);
				if (ImGui::Button("Z")) value->z = resetValue;
				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);
				ImGui::SameLine();
				ImGui::DragFloat("##Z", &value->z, dragSpeed, min, max, format);

				ImGui::PopStyleVar();
				ImGui::EndTable();
			}


			ImGui::EndTable();
		}
		ImGui::PopID();
	}
	void Vector3DragOld(const char *label, glm::vec3 &value, const char* format, float resetValue, float dragSpeed) {
		float min = 0.f;
		float max = 0.f;

		float columnWidth = 75.f;

		ImGui::PushID(label);

		ImGui::Columns(2);
		// just the space for the text that is represented below
		float width = (columnWidth > 0) ? columnWidth : ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemSpacing.x;
		ImGui::SetColumnWidth(0, width);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", label);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{1, 0}); // spacing between the X, Y, Z and the actual values
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // the border around the buttons

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 1.0f; // magic number here
		ImVec2 buttonSize = {ImGui::CalcTextSize("X").x*2, lineHeight};
		ImVec2 framePadding = { ImGui::CalcTextSize("X").x/2, 0.0f };

		ImGuizmo::Style guizmostyle  = ImGuizmo::GetStyle();
		ImVec4* guizmocolors = guizmostyle.Colors;
		// float 3 drawing below
		ImGui::PushStyleColor(ImGuiCol_Button, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_X], -0.15f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_X], 0.25f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, guizmocolors[ImGuizmo::DIRECTION_X]);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		if (ImGui::Button("X", buttonSize)) value.x = resetValue;
		ImGui::PopStyleVar();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &value.x, dragSpeed, min, max, format);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Y], -0.15f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Y], 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, guizmocolors[ImGuizmo::DIRECTION_Y]);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		if (ImGui::Button("Y", buttonSize)) value.y = resetValue;
		ImGui::PopStyleVar();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &value.y, dragSpeed, min, max, format);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Z], -0.15f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Adjust_Saturation(guizmocolors[ImGuizmo::DIRECTION_Z], 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, guizmocolors[ImGuizmo::DIRECTION_Z]);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
		if (ImGui::Button("Z", buttonSize)) value.z = resetValue;
		ImGui::PopStyleVar();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &value.z, dragSpeed, min, max, format);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopStyleVar();

		ImGui::PopID();
	}
	void HighlightedText(const char* text, ImVec4 bg_color, ImVec2 padding, ImVec4 text_color) {
		ImGui::PushStyleColor(ImGuiCol_Text, text_color);
		ImVec2 text_size = ImGui::CalcTextSize(text);
		ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

		// Add padding around the text
		float rect_min_x = cursor_pos.x - padding.x;
		float rect_min_y = cursor_pos.y - padding.y;
		float rect_max_x = cursor_pos.x + text_size.x + padding.x;
		float rect_max_y = cursor_pos.y + text_size.y + padding.y;

		// DrawMeshData_EXT the background rectangle
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(
				ImVec2(rect_min_x, rect_min_y),
				ImVec2(rect_max_x, rect_max_y),
				ImGui::GetColorU32(bg_color),
				0.0f // Rounded corners (optional)
		);

		ImGui::TextUnformatted(text);
		ImGui::PopStyleColor();
	}
}