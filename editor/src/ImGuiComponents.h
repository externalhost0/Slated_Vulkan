//
// Created by Hayden Rivas on 2/4/25.
//
// the goal of SlateGui is to provide a wrapper around many of the ImGui-
// widgets that better suites the style of the gui i want
#pragma once
#include <span>
#include <glm/glm.hpp>

enum SlateGuiColorEditFlags_
{
	SlateGuiColorEditFlags_None = 0,

};


namespace SlateGui {


	bool ColorField(glm::vec3& color, const char* label = nullptr, SlateGuiColorEditFlags_ flags = SlateGuiColorEditFlags_None);
	bool ColorField(glm::vec4& color, const char* label = nullptr, SlateGuiColorEditFlags_ flags = SlateGuiColorEditFlags_None);






}


