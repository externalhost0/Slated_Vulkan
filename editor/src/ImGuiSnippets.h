//
// Created by Hayden Rivas on 1/22/25.
//

#pragma once
#include "Slate/ECS/Components.h"
#include "Slate/ECS/Entity.h"

#include <IconFontCppHeaders/IconsLucide.h>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <string>

#include "Fonts.h"

namespace Slate {
	ImVec4 Brighten(const ImVec4& color, float amount);
	ImVec4 Desaturate(const ImVec4& color, float amount);
	ImVec4 HueShift(const ImVec4& color, float amount);


	// short imgui widhets
	void Vector3Drag(const char* label, glm::vec3* value, const char* format, float resetValue, float dragSpeed);
	void HighlightedText(const char* text, ImVec4 bg_color, ImVec2 padding = {0.0f, 0.0f}, ImVec4 text_color = {1, 1, 1, 1});
}