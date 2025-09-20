//
// Created by Hayden Rivas on 1/8/25.
//

// external headers

#include <volk.h>
#include <imgui_impl_vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>


#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <span>
#include <chrono>
#include <iostream>

// Slate Headers
#include "Slate/Common/HelperMacros.h"
#include "Slate/Common/Logger.h"
#include "Slate/ECS/Components.h"
#include "Slate/ECS/Entity.h"
#include <Slate/Filesystem.h>
#include <Slate/Loaders/GLTFLoader.h>
#include <Slate/MeshGenerators.h>
#include <Slate/PipelineBuilder.h>
#include <Slate/Primitives.h>
#include <Slate/Resources/MeshResource.h>
#include <Slate/Resources/ShaderResource.h>
#include <Slate/SceneTemplates.h>
#include <Slate/VK/vkinfo.h>
#include <Slate/Window.h>

// editor headers
#include "Editor.h"
#include "Fonts.h"
#include "ImGuiSnippets.h"

#include "Slate/CommandBuffer.h"
#include "Slate/Loaders/ImageLoader.h"
#include "Slate/Resources/TextureResource.h"
#include "Slate/Systems/ShaderSystem.h"
#include "Slate/VkObjects.h"


#include "Slate/Network/NamedPipe.h"
#include "imgui_internal.h"

#include "Slate/Network/Socket.h"
#include "generated/engine.pb.h"
#include <zpp_bits.h>

// icons!
#include <IconFontCppHeaders/IconsLucide.h>
#include <nfd.h>

namespace Slate {

	static void glfw_error_callback(int error, const char *description) {
		fprintf(stderr, "[GLFW] Error %d: %s\n", error, description);
	}

	void FontSetup() {
		ImGuiIO &io = ImGui::GetIO();
		// font size controls everything
		float fontSize = 17.f;

// for crisp text on mac retina displays
#if defined(SLATE_OS_MACOS)
		fontSize *= 2.f;
		io.FontGlobalScale = 1.0f / 2.0f;
#else
		io.FontGlobalScale = 1.0F;
#endif
		// main font config, for retina displays
		ImFontConfig fontCfg = {};
		{
			fontCfg.FontDataOwnedByAtlas = false;
			fontCfg.OversampleV = 2;
			fontCfg.OversampleH = 1;
			fontCfg.RasterizerDensity = 1.0f;
			fontCfg.GlyphOffset = ImVec2(-0.4f, 0.0f);
		}
		// icon fonts, how much should we scale the icons
		float iconSize = fontSize * 0.89f;
		ImFontConfig iconFontCfg = {};
		{
			iconFontCfg.MergeMode = true;
			iconFontCfg.GlyphMinAdvanceX = iconSize;                 // Use if you want to make the icon monospaced
			iconFontCfg.GlyphOffset = ImVec2(1.0f, 5.f);// fixes the offset in the text
			iconFontCfg.PixelSnapH = true;
		}

		static const ImWchar ILC_Range[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};

		// the path in which all the fonts are located, figure out a better way to set this later
		const std::string path = Filesystem::GetRelativePath("fonts/");
		// main font, also merged with below icons
		io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize, &iconFontCfg, ILC_Range);

		float mediumSize = 3.0f;
		Fonts::iconMediumFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize + mediumSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize + mediumSize, &iconFontCfg, ILC_Range);

		float largeSize = 10.0f;
		Fonts::iconLargeFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize + largeSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize + largeSize, &iconFontCfg, ILC_Range);

		float extraLargeSize = largeSize + 8.f;
		Fonts::iconExtraLargeFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize + extraLargeSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize + extraLargeSize, &iconFontCfg, ILC_Range);

		// variants of the NotoSans main font
		// must be after the main font as we merge the fonts with the last font that is added to the io
		Fonts::boldFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Bold.ttf").c_str(), fontSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize, &iconFontCfg, ILC_Range);

		Fonts::largeboldFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Bold.ttf").c_str(), fontSize + 5.0f, &fontCfg);
		Fonts::italicFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Italic.ttf").c_str(), fontSize, &fontCfg);
		Fonts::largeitalicFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Italic.ttf").c_str(), fontSize + 5.0f, &fontCfg);
	}

	namespace Theme {
		static constexpr float MAIN_COLOR = (0.f);
	}
	struct ImGuiThemeProfile {
		ImVec4 baseBg = ImVec4(0.10f, 0.10f, 0.11f, 1.0f);
		ImVec4 accent = ImVec4(0.30f, 0.50f, 0.95f, 1.0f);
		ImVec4 text   = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);

		float contrast = 0.05f;
		float dimness  = 0.15f;
		float borderStrength = 0.4f;
	};


	void ColorStyleLightBlue() {
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;
		{
			colors[ImGuiCol_Text]                 = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
			colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg]             = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
			colors[ImGuiCol_ChildBg]              = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
			colors[ImGuiCol_PopupBg]              = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);

			colors[ImGuiCol_Border]               = ImVec4(0.26f, 0.26f, 0.29f, 0.60f);
			colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

			colors[ImGuiCol_FrameBg]              = ImVec4(0.20f, 0.21f, 0.25f, 1.00f);
			colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.30f, 0.31f, 0.36f, 1.00f);
			colors[ImGuiCol_FrameBgActive]        = ImVec4(0.36f, 0.38f, 0.45f, 1.00f);

			colors[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
			colors[ImGuiCol_TitleBgActive]        = ImVec4(0.15f, 0.16f, 0.19f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.08f, 0.09f, 0.10f, 1.00f);

			colors[ImGuiCol_MenuBarBg]            = ImVec4(0.15f, 0.16f, 0.19f, 1.00f);

			colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.12f, 0.50f);
			colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.25f, 0.30f, 0.70f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.35f, 0.80f);
			colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);

			colors[ImGuiCol_CheckMark]            = ImVec4(0.20f, 0.80f, 0.60f, 1.00f);

			colors[ImGuiCol_SliderGrab]           = ImVec4(0.28f, 0.56f, 0.98f, 0.75f);
			colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.28f, 0.56f, 0.98f, 1.00f);

			colors[ImGuiCol_Button]               = ImVec4(0.20f, 0.21f, 0.25f, 1.00f);
			colors[ImGuiCol_ButtonHovered]        = ImVec4(0.28f, 0.29f, 0.35f, 1.00f);
			colors[ImGuiCol_ButtonActive]         = ImVec4(0.33f, 0.35f, 0.42f, 1.00f);

			colors[ImGuiCol_Header]               = ImVec4(0.25f, 0.27f, 0.32f, 1.00f);
			colors[ImGuiCol_HeaderHovered]        = ImVec4(0.30f, 0.32f, 0.38f, 1.00f);
			colors[ImGuiCol_HeaderActive]         = ImVec4(0.35f, 0.37f, 0.44f, 1.00f);

			colors[ImGuiCol_Separator]            = ImVec4(0.25f, 0.27f, 0.30f, 1.00f);
			colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.30f, 0.32f, 0.35f, 1.00f);
			colors[ImGuiCol_SeparatorActive]      = ImVec4(0.35f, 0.37f, 0.40f, 1.00f);

			colors[ImGuiCol_ResizeGrip]           = ImVec4(0.25f, 0.27f, 0.30f, 0.70f);
			colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.30f, 0.32f, 0.35f, 0.80f);
			colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.35f, 0.37f, 0.40f, 1.00f);

			colors[ImGuiCol_Tab]                  = ImVec4(0.18f, 0.20f, 0.25f, 0.86f);
			colors[ImGuiCol_TabHovered]           = ImVec4(0.28f, 0.56f, 0.98f, 0.80f);

			colors[ImGuiCol_PlotLines]            = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram]        = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

			colors[ImGuiCol_TableHeaderBg]        = ImVec4(0.19f, 0.20f, 0.23f, 1.00f);
			colors[ImGuiCol_TableBorderStrong]    = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
			colors[ImGuiCol_TableBorderLight]     = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
			colors[ImGuiCol_TableRowBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

			colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.28f, 0.56f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget]       = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavCursor]            = ImVec4(0.28f, 0.56f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
			colors[ImGuiCol_DockingPreview]       = ImVec4(0.28f, 0.56f, 0.98f, 0.70f);
		}
	}
	void ColorStylePurple() {
		ImGui::StyleColorsDark();
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		// Base tones
		colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);
		colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
		colors[ImGuiCol_WindowBg]             = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
		colors[ImGuiCol_ChildBg]              = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg]              = ImVec4(0.14f, 0.14f, 0.17f, 1.00f);

		// Borders
		colors[ImGuiCol_Border]               = ImVec4(0.35f, 0.36f, 0.40f, 0.60f);
		colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Frame & Widgets
		colors[ImGuiCol_FrameBg]              = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.36f, 0.14f, 0.44f, 1.00f);
		colors[ImGuiCol_FrameBgActive]        = ImVec4(0.46f, 0.20f, 0.54f, 1.00f);

		colors[ImGuiCol_TitleBg]              = ImVec4(0.15f, 0.10f, 0.20f, 1.00f);
		colors[ImGuiCol_TitleBgActive]        = ImVec4(0.25f, 0.15f, 0.35f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.07f, 0.13f, 1.00f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.12f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.33f, 0.33f, 0.36f, 0.71f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.44f, 0.14f, 0.56f, 0.80f);
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.20f, 0.60f, 1.00f);

		// Interactive elements
		colors[ImGuiCol_CheckMark]            = ImVec4(0.90f, 0.40f, 0.95f, 1.00f);
		colors[ImGuiCol_SliderGrab]           = ImVec4(0.80f, 0.30f, 0.90f, 0.75f);
		colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.90f, 0.40f, 0.95f, 1.00f);

		colors[ImGuiCol_Button]               = ImVec4(0.25f, 0.27f, 0.30f, 1.00f);
		colors[ImGuiCol_ButtonHovered]        = ImVec4(0.40f, 0.14f, 0.50f, 1.00f);
		colors[ImGuiCol_ButtonActive]         = ImVec4(0.48f, 0.20f, 0.56f, 1.00f);

		// Headers
		colors[ImGuiCol_Header]               = ImVec4(0.30f, 0.12f, 0.42f, 1.00f);
		colors[ImGuiCol_HeaderHovered]        = ImVec4(0.38f, 0.15f, 0.50f, 1.00f);
		colors[ImGuiCol_HeaderActive]         = ImVec4(0.46f, 0.18f, 0.58f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab]                  = ImVec4(0.22f, 0.20f, 0.30f, 0.80f);
		colors[ImGuiCol_TabHovered]           = ImVec4(0.40f, 0.14f, 0.50f, 0.85f);
//		colors[ImGuiCol_TabActive]            = ImVec4(0.45f, 0.20f, 0.55f, 1.00f);
//		colors[ImGuiCol_TabUnfocused]         = ImVec4(0.16f, 0.16f, 0.20f, 0.90f);
//		colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.25f, 0.18f, 0.35f, 1.00f);

		// Separators
		colors[ImGuiCol_Separator]            = ImVec4(0.35f, 0.20f, 0.40f, 0.60f);
		colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.45f, 0.25f, 0.55f, 1.00f);
		colors[ImGuiCol_SeparatorActive]      = ImVec4(0.55f, 0.30f, 0.65f, 1.00f);

		// Plot
		colors[ImGuiCol_PlotLines]            = ImVec4(0.75f, 0.60f, 0.85f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]     = ImVec4(0.95f, 0.70f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram]        = ImVec4(0.70f, 0.30f, 0.80f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.85f, 0.40f, 0.95f, 1.00f);

		// Tables
		colors[ImGuiCol_TableHeaderBg]        = ImVec4(0.22f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_TableBorderStrong]    = ImVec4(0.45f, 0.20f, 0.50f, 1.00f);
		colors[ImGuiCol_TableBorderLight]     = ImVec4(0.35f, 0.15f, 0.45f, 1.00f);
		colors[ImGuiCol_TableRowBg]           = ImVec4(0.10f, 0.10f, 0.12f, 0.30f);
		colors[ImGuiCol_TableRowBgAlt]        = ImVec4(0.12f, 0.12f, 0.15f, 0.30f);

		// Misc
		colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.90f, 0.50f, 1.00f, 0.35f);
		colors[ImGuiCol_DragDropTarget]       = ImVec4(1.00f, 0.80f, 0.10f, 0.90f);
		colors[ImGuiCol_NavCursor]            = ImVec4(1.00f, 0.75f, 1.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colors[ImGuiCol_DockingPreview]       = ImVec4(0.95f, 0.70f, 1.00f, 0.30f);
		// Resize grip styles (bottom-right corner of windows)
		colors[ImGuiCol_ResizeGrip]           = ImVec4(0.40f, 0.18f, 0.50f, 0.45f);
		colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.55f, 0.28f, 0.65f, 0.80f);
		colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.70f, 0.38f, 0.80f, 1.00f);

		// Tab styling extensions
		colors[ImGuiCol_TabSelected]          = ImVec4(0.55f, 0.30f, 0.65f, 1.00f); // Strongest tab state
		colors[ImGuiCol_TabSelectedOverline]          = ImVec4(0.90f, 0.50f, 1.00f, 1.00f); // Line above selected tab (if supported)
		colors[ImGuiCol_TabDimmed]            = ImVec4(0.20f, 0.18f, 0.25f, 0.60f); // For minimized/inactive tabs
		colors[ImGuiCol_TabDimmedSelected]    = ImVec4(0.35f, 0.20f, 0.45f, 0.85f); // Dimmed but active (like preview state)

		// Text link styling (underlined text or hyperlink)
		colors[ImGuiCol_TextLink]             = ImVec4(0.75f, 0.50f, 1.00f, 1.00f); // Vibrant link color

		// Cursor feedback color (hovering on nav via gamepad/keyboard)

	}
	void ApplyTheme(const ImGuiThemeProfile& profile) {
		ImVec4* colors = ImGui::GetStyle().Colors;

		// Base backgrounds
		colors[ImGuiCol_WindowBg]   = profile.baseBg;
		colors[ImGuiCol_ChildBg]    = Adjust_Brightness(profile.baseBg, 0.03f);
		colors[ImGuiCol_PopupBg]    = Adjust_Brightness(profile.baseBg, 0.05f);
		colors[ImGuiCol_MenuBarBg]  = Adjust_Brightness(Adjust_Saturation(profile.baseBg, 0.2f), 0.05f);

		// Text
		colors[ImGuiCol_Text]              = profile.text;
		colors[ImGuiCol_TextDisabled]      = Adjust_Saturation(profile.text, profile.dimness);
		colors[ImGuiCol_TextLink]          = Adjust_Brightness(Adjust_HueShift(profile.accent, 0.1f), 0.1f);
		colors[ImGuiCol_TextSelectedBg]    = Adjust_Brightness(profile.accent, 0.2f);

		// Frames
		colors[ImGuiCol_FrameBg]           = Adjust_Brightness(profile.baseBg, 0.04f);
		colors[ImGuiCol_FrameBgHovered]    = Adjust_Brightness(profile.accent, profile.contrast);
		colors[ImGuiCol_FrameBgActive]     = Adjust_Brightness(profile.accent, profile.contrast * 2.0f);

		// Titles
		colors[ImGuiCol_TitleBg]          = Adjust_Brightness(profile.baseBg, 0.03f);
		colors[ImGuiCol_TitleBgActive]    = Adjust_Brightness(Adjust_HueShift(profile.baseBg, 0.02f), 0.08f);
		colors[ImGuiCol_TitleBgCollapsed] = Adjust_Saturation(profile.baseBg, 0.1f);

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg]          = Adjust_Saturation(profile.baseBg, 0.5f);
		colors[ImGuiCol_ScrollbarGrab]        = Adjust_Brightness(profile.baseBg, 0.2f);
		colors[ImGuiCol_ScrollbarGrabHovered] = Adjust_Brightness(profile.accent, 0.15f);
		colors[ImGuiCol_ScrollbarGrabActive]  = Adjust_Brightness(profile.accent, 0.25f);

		// Buttons
		colors[ImGuiCol_Button]            = Adjust_Brightness(profile.baseBg, 0.07f);
		colors[ImGuiCol_ButtonHovered]     = Adjust_Brightness(profile.accent, profile.contrast);
		colors[ImGuiCol_ButtonActive]      = Adjust_Brightness(profile.accent, profile.contrast * 1.5f);

		// Headers
		colors[ImGuiCol_Header]        = Adjust_Brightness(Adjust_Saturation(profile.accent, 0.5f), -0.5f);
		colors[ImGuiCol_HeaderHovered] = Adjust_Brightness(Adjust_Saturation(profile.accent, 0.3f), 0.1f);
		colors[ImGuiCol_HeaderActive]  = Adjust_Brightness(profile.accent, profile.contrast * 0.6f);

		// Tabs
		colors[ImGuiCol_Tab]                        = Adjust_Saturation(profile.baseBg, 0.25f);
		colors[ImGuiCol_TabHovered]                 = Adjust_Brightness(profile.accent, profile.contrast * 0.8f);
		colors[ImGuiCol_TabSelected]                = Adjust_Brightness(profile.baseBg, 0.05f);
		colors[ImGuiCol_TabSelectedOverline]        = Adjust_Brightness(Adjust_HueShift(profile.accent, 0.1f), 0.3f);
		colors[ImGuiCol_TabDimmed]                  = Adjust_Saturation(profile.baseBg, 0.4f);
		colors[ImGuiCol_TabDimmedSelected]          = Adjust_Brightness(profile.baseBg, 0.02f);
		colors[ImGuiCol_TabDimmedSelectedOverline]  = Adjust_Brightness(Adjust_HueShift(profile.accent, 0.15f), 0.15f);

		// Sliders, checkmarks, grabbers
		colors[ImGuiCol_SliderGrab]        = profile.accent;
		colors[ImGuiCol_SliderGrabActive]  = Adjust_Brightness(profile.accent, profile.contrast);
		colors[ImGuiCol_CheckMark]         = profile.accent;

		// Resize grips
		colors[ImGuiCol_ResizeGrip]        = Adjust_Saturation(profile.accent, 0.4f);
		colors[ImGuiCol_ResizeGripHovered] = Adjust_Brightness(profile.accent, 0.2f);
		colors[ImGuiCol_ResizeGripActive]  = Adjust_Brightness(profile.accent, 0.3f);

		// Navigation highlight
		colors[ImGuiCol_NavCursor]             = Adjust_Brightness(profile.accent, 0.4f);
		colors[ImGuiCol_NavWindowingHighlight] = Adjust_Brightness(profile.accent, 0.4f);
		colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0, 0, 0, 0.3f);

		// Tables
		colors[ImGuiCol_TableHeaderBg]     = Adjust_Brightness(profile.baseBg, 0.06f);
		colors[ImGuiCol_TableBorderStrong] = Adjust_Brightness(profile.baseBg, profile.borderStrength);
		colors[ImGuiCol_TableBorderLight]  = Adjust_Brightness(profile.baseBg, profile.borderStrength * 0.5f);
		colors[ImGuiCol_TableRowBg]        = profile.baseBg;
		colors[ImGuiCol_TableRowBgAlt]     = Adjust_Brightness(profile.baseBg, 0.03f);

		// Misc
		colors[ImGuiCol_DockingPreview]    = Adjust_Brightness(profile.accent, 0.2f);
		colors[ImGuiCol_DragDropTarget]    = Adjust_Brightness(profile.accent, 0.4f);
		colors[ImGuiCol_ModalWindowDimBg]  = ImVec4(0, 0, 0, 0.6f);

		// Seperators
		colors[ImGuiCol_Separator]         = Adjust_Brightness(profile.baseBg, 0.08f);
		colors[ImGuiCol_SeparatorHovered]  = Adjust_Brightness(profile.accent, profile.contrast * 0.8f);
		colors[ImGuiCol_SeparatorActive]   = Adjust_Brightness(profile.accent, profile.contrast * 1.2f);
	}
	void ColorStyleGuizmo() {
		ImGuizmo::Style* guizmostyle = &ImGuizmo::GetStyle();
		float thickness = 4.0f;
		{
			guizmostyle->TranslationLineThickness = thickness;
			guizmostyle->RotationLineThickness = thickness - 2.0f;
			guizmostyle->ScaleLineThickness = thickness;
			guizmostyle->TranslationLineArrowSize = 7.0f;
			guizmostyle->RotationOuterLineThickness = thickness - 1.0f;
			guizmostyle->HatchedAxisLineThickness = 1.0f;
		}
		ImVec4* guizmocolors = guizmostyle->Colors;
		{
			guizmocolors[ImGuizmo::DIRECTION_X] = ImVec4(0.858f, 0.243f, 0.113f, 0.929f);
			guizmocolors[ImGuizmo::DIRECTION_Y] = ImVec4(0.603f, 0.952f, 0.282f, 0.929f);
			guizmocolors[ImGuizmo::DIRECTION_Z] = ImVec4(0.227f, 0.478f, 0.972f, 0.929f);
			guizmocolors[ImGuizmo::ROTATION_USING_FILL] = ImVec4(0.727f, 0.478f, 0.072f, 1.0f);
			guizmocolors[ImGuizmo::ROTATION_USING_BORDER] = ImVec4(0.5f, 0.3f, 0.05f, 0.8f);
		}
	}
	void StyleStandard(ImGuiStyle *dst = nullptr) {
		ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
		{
			style->DockingSeparatorSize = 1.0f;
			style->FrameBorderSize = 1.0f;
			style->FramePadding = ImVec2(10.0f, 4.0f);

			style->TabBarBorderSize = 2.0f;
			style->TabBarOverlineSize = 1.0f;
			style->WindowBorderSize = 1.0f; // for the thick borders on everything
			style->PopupBorderSize = 1.0f; // for those nice borders around things like menus
			style->ScrollbarSize = 13.0f;

			style->WindowRounding = 2.0f;
			style->ScrollbarRounding = 0.0f;
			style->GrabRounding = 1.0f;
			style->FrameRounding = 1.0f;
			style->ChildRounding = 2.0f;
			style->TabRounding = 1.5f;
		}
		ImGuiThemeProfile VioletVoid = {
				.baseBg = ImVec4(0.08f, 0.06f, 0.10f, 1.0f),
				.accent = ImVec4(0.75f, 0.40f, 1.00f, 1.0f),
				.text   = ImVec4(0.93f, 0.94f, 0.96f, 1.0f),
				.contrast = 0.18f,
				.dimness  = 0.28f,
				.borderStrength = 0.35f
		};
		ImGuiThemeProfile SlateNeon = {
				.baseBg = ImVec4(0.10f, 0.12f, 0.14f, 1.0f),
				.accent = ImVec4(0.20f, 0.80f, 0.95f, 1.0f), // neon cyan
				.text   = ImVec4(0.95f, 0.96f, 0.98f, 1.0f),
				.contrast = 0.20f,
				.dimness  = 0.25f,
				.borderStrength = 0.4f
		};
		ImGuiThemeProfile EmeraldNight = {
				.baseBg = ImVec4(0.09f, 0.11f, 0.10f, 1.0f),
				.accent = ImVec4(0.25f, 1.00f, 0.70f, 1.0f),
				.text   = ImVec4(0.92f, 0.96f, 0.92f, 1.0f),
				.contrast = 0.22f,
				.dimness  = 0.3f,
				.borderStrength = 0.35f
		};
		ImGuiThemeProfile SunsetTerminal = {
				.baseBg = ImVec4(0.12f, 0.08f, 0.06f, 1.0f),
				.accent = ImVec4(1.00f, 0.55f, 0.25f, 1.0f), // amber
				.text   = ImVec4(1.00f, 0.95f, 0.85f, 1.0f),
				.contrast = 0.17f,
				.dimness  = 0.35f,
				.borderStrength = 0.45f
		};
		ImGuiThemeProfile FrostLight = {
				.baseBg = ImVec4(0.90f, 0.92f, 0.95f, 1.0f),
				.accent = ImVec4(0.15f, 0.50f, 0.80f, 1.0f),
				.text   = ImVec4(0.10f, 0.12f, 0.15f, 1.0f),
				.contrast = 0.12f,
				.dimness  = 0.4f,
				.borderStrength = 0.2f
		};

		ApplyTheme(SunsetTerminal);
//		ColorStyleLightBlue();
//		ColorStylePurple();
		ColorStyleGuizmo();
	}
	void BuildStyle() {
		// necessary to be done early
		FontSetup();
		// our default colors for the user interface
		StyleStandard();
	}



	glm::mat4 TransformToModelMatrix(const TransformComponent &component, bool isScalable = true, bool isRotatable = true) {
		auto model = glm::mat4(1);
		model = glm::translate(model, component.global.position);
		// some models we dont want scale to affect them, this is mostly for editor visualizers
		if (isRotatable) model = model * glm::mat4_cast(component.global.rotation);
		if (isScalable) model = glm::scale(model, component.global.scale);
		return model;
	}
	glm::mat4 BillboardModelMatrix(glm::mat4 model, ViewportCamera &camera) {
		// get camera properties using helper functions
		glm::vec3 cameraPosition = camera.getPosition();
		glm::vec3 cameraUp = camera.getUpVector();
		glm::vec3 cameraForward = camera.getFrontVector();

		glm::vec3 right = glm::normalize(glm::cross(cameraUp, cameraForward));
		glm::vec3 up = glm::cross(cameraForward, right);

		glm::vec3 modelPosition = model[3];
		// apply new rotation basis to model matrix (overwrite rotation)
		model[0] = glm::vec4(right, 0.0f);
		model[1] = glm::vec4(-up, 0.0f);
		model[2] = glm::vec4(-cameraForward, 0.0f);
		model[3] = glm::vec4(modelPosition, 1.0f);// preserve position
		return model;
	}

	// recursive
//	void Editor::DrawEntity(VkCommandBuffer cmd, GameEntity entity, VkPipelineLayout layout, const glm::mat4& topMatrix) {
//		if (entity.HasComponent<GeometryPrimitiveComponent>()) {
//			const GeometryPrimitiveComponent& mesh_component = entity.GetComponent<GeometryPrimitiveComponent>();
//			if (mesh_component.mesh_type == MeshPrimitiveType::Empty) return;
//			GPU::DrawPushConstants push = {};
//			push.id = static_cast<uint32_t>(entity.GetHandle());
//			push.modelMatrix = TransformToModelMatrix(entity.GetComponent<TransformComponent>()) * topMatrix;
//
//			RenderBuffer(cmd, this->defaultMeshPrimitiveTypes[mesh_component.mesh_type], push, layout);
//		} else if (entity.HasComponent<GeometryGLTFComponent>()) {
//			const GeometryGLTFComponent& mesh_component = entity.GetComponent<GeometryGLTFComponent>();
//			GPU::DrawPushConstants push = {};
//			push.id = static_cast<uint32_t>(entity.GetHandle());
//			push.modelMatrix = TransformToModelMatrix(entity.GetComponent<TransformComponent>()) * topMatrix;
//
////			for (const StrongPtr<MeshBuffer>& buffer: mesh_component.GetMesh()->GetMeshBuffers()) {
////				RenderBuffer(cmd, buffer, push, layout);
////			}
//		}
//
//		// now recurse through all children if children are present
//		if (!entity.HasChildren()) return;
//		for (GameEntity child : entity.GetChildren()) {
//			DrawEntity(cmd, child, layout, TransformToModelMatrix(entity.GetComponent<TransformComponent>()));
//		}
//	}
//
//
//	// recursive
//	void Editor::DrawEntityForEditorEXT(VkCommandBuffer cmd, GameEntity entity, VkPipelineLayout layout, const glm::mat4& topMatrix) {
//		if (entity.HasComponent<GeometryPrimitiveComponent>()) {
//			const GeometryPrimitiveComponent& mesh_component = entity.GetComponent<GeometryPrimitiveComponent>();
//			if (mesh_component.mesh_type == MeshPrimitiveType::Empty) return;
//			GPU::DrawPushConstantsEditorEXT push = {};
//			push.id = static_cast<uint32_t>(entity.GetHandle());
//			push.modelMatrix = TransformToModelMatrix(entity.GetComponent<TransformComponent>()) * topMatrix;
//			push.color = glm::vec3(1.f, 0.706f, 0.f);
//
//			const MeshBuffer& buffer = this->defaultMeshPrimitiveTypes[mesh_component.mesh_type];
//			push.vertexBufferAddress = buffer.GetVBA();
//			vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstantsEditorEXT), &push);
//			vkCmdBindIndexBuffer(cmd, buffer.GetIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
//			vkCmdDrawIndexed(cmd, buffer.GetIndexCount(), 1, 0, 0, 0);
//		} else if (entity.HasComponent<GeometryGLTFComponent>()) {
//			const GeometryGLTFComponent& mesh_component = entity.GetComponent<GeometryGLTFComponent>();
//			GPU::DrawPushConstantsEditorEXT push = {};
//			push.id = static_cast<uint32_t>(entity.GetHandle());
//			push.modelMatrix = TransformToModelMatrix(entity.GetComponent<TransformComponent>()) * topMatrix;
//			push.color = glm::vec3(0.9f);
//
////			for (const auto &buffer: mesh_component.GetMesh()->GetMeshBuffers()) {
////				push.vertexBufferAddress = buffer->getVBA();
////				vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstantsEditorEXT), &push);
////				vkCmdBindIndexBuffer(cmd, buffer->GetIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
////				vkCmdDrawIndexed(cmd, buffer->getIndexCount(), 1, 0, 0, 0);
////			}
//		}
//
//		// now recurse through all children if children are present
//		if (!entity.HasChildren()) return;
//		for (GameEntity child: entity.GetChildren()) {
//			DrawEntity(cmd, child, layout, TransformToModelMatrix(entity.GetComponent<TransformComponent>()));
//		}
//	}
//	// recursive
//	void Editor::DrawEntityForEditorLarge_EXT(VkCommandBuffer cmd, GameEntity entity, VkPipelineLayout layout, const glm::mat4 &topMatrix) {
//		if (entity.HasComponent<GeometryPrimitiveComponent>()) {
//			const GeometryPrimitiveComponent& mesh_component = entity.GetComponent<GeometryPrimitiveComponent>();
//			if (mesh_component.mesh_type == MeshPrimitiveType::Empty) return;
//			GPU::DrawPushConstantsEditorEXT push = {};
//			const MeshBuffer& buffer = this->defaultMeshPrimitiveTypes[mesh_component.mesh_type];
//			push.vertexBufferAddress = buffer.GetVBA();
//			push.modelMatrix = glm::scale(TransformToModelMatrix(entity.GetComponent<TransformComponent>()) * topMatrix, glm::vec3(1.0005f));
//			push.color = glm::vec3(1.f, 0.706f, 0.f);
//
//			vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstantsEditorEXT), &push);
//			vkCmdBindIndexBuffer(cmd, buffer.GetIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
//			vkCmdDrawIndexed(cmd, buffer.GetIndexCount(), 1, 0, 0, 0);
//		} else if (entity.HasComponent<GeometryGLTFComponent>()) {
//			const GeometryGLTFComponent& mesh_component = entity.GetComponent<GeometryGLTFComponent>();
//			GPU::DrawPushConstantsEditorEXT push = {};
//			glm::mat4 adjpos = TransformToModelMatrix(entity.GetComponent<TransformComponent>()) * topMatrix;
//
////			for (const auto &buffer: mesh_component.GetMesh()->GetMeshBuffers()) {
////				push.vertexBufferAddress = buffer->getVBA();
////				push.modelMatrix = glm::scale(adjpos, glm::vec3(1.0005f));
////				push.color = glm::vec3(0.9f);
////
////				vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstantsEditorEXT), &push);
////				vkCmdBindIndexBuffer(cmd, buffer->GetIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
////				vkCmdDrawIndexed(cmd, buffer->getIndexCount(), 1, 0, 0, 0);
////			}
//		}
//
//		// now recurse through all children if children are present
//		if (!entity.HasChildren()) return;
//		for (GameEntity child : entity.GetChildren()) {
//			DrawEntityForEditorLarge_EXT(cmd, child, layout, TransformToModelMatrix(entity.GetComponent<TransformComponent>()));
//		}
//	}

	void EditorApplication::InitImGui(ImGuiRequiredData req, GLFWwindow* glfwWindow, VkFormat format) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
		io.ConfigDragClickToInputText = true;                // makes single click on sldiers
		io.ConfigWindowsMoveFromTitleBarOnly = true;         // makes it so you can only move windows from the bar, required for viewport functionality when undocked
		io.ConfigInputTextCursorBlink = true;                // enables blinking cursor in text boxes
#if defined(SLATE_OS_MACOS)
		io.ConfigMacOSXBehaviors = true;// changes a ton of stuff, just click on it
#endif
		// init imgui for glfw
		VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
											 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
											 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
											 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;
		VK_CHECK(vkCreateDescriptorPool(req.device, &pool_info, nullptr, &_imguiDescriptorPool));

		ImGui_ImplGlfw_InitForVulkan(glfwWindow, false);

		// vulkan render info from our rendersystem
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = req.instance;
		initInfo.PhysicalDevice = req.physdevice;
		initInfo.Device = req.device;
		initInfo.Queue = req.graphicsqueue;
		initInfo.DescriptorPool = _imguiDescriptorPool;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		// dynamic rendering requirements
		initInfo.UseDynamicRendering = true;

		initInfo.PipelineRenderingCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = nullptr,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &format};
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = [](VkResult err) {
			if (err == 0) return;
		  	fprintf(stderr, "[Vulkan] Error: VkResult = %d\n", err);
		  	if (err < 0) abort();
		};
		// init imgui for vulkan
		ImGui_ImplVulkan_Init(&initInfo);
	}

	void EditorApplication::onWindowMinimize() {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	InternalPipelineHandle shadedModePipeline;
	InternalPipelineHandle wireframeModePipeline;
	InternalPipelineHandle unshadedModePipeline;
	InternalPipelineHandle gridShaderPipeline;

	InternalPipelineHandle wireframeVisualizerPipeline;
	InternalPipelineHandle filledVisualizerPipeline;
	InternalPipelineHandle fullscreenPipeline;
	InternalPipelineHandle pureOutlinePipeline;

	// editor provided resources

	MeshData quadMeshData;
	MeshData planeMeshData;
	MeshData cubeMeshData;
	MeshData sphereMeshData;


	// visible built in editor resources
	TextureResource lightbulbTexture;
	TextureResource sunTexture;
	TextureResource spotlightTexture;
	TextureResource cubemapRes;

	ShaderResource standardShader;
	ShaderResource primitiveShader;
	ShaderResource imageShader;
	ShaderResource solidShader;
	ShaderResource infiniteGridShader;
	ShaderResource fullscreenShader;
	ShaderResource pureMaskShader;


	void CreateEditorAttachments(GX& gx, EditorApplication& app) {
		const VkExtent2D extent2D = gx.getSwapchainExtent();

		app.colorResolveImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Sampled | TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.debugName = "Color Resolve Image"
		});
		app.colorMSAAImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X4,
				.usage = TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Memoryless,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.debugName = "Color MSAA Image"
		});
		app.depthStencilMSAAImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X4,
				.usage = TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Memoryless,
				.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.debugName = "DepthStencil Image"
		});
		app.entityResolveImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Attachment | TextureUsageBits::TextureUsageBits_Storage,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R32_UINT,
				.debugName = "Entity Image"
		});
		app.entityMSAAImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X4,
				.usage = TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Memoryless,
				.format = VK_FORMAT_R32_UINT,
				.debugName = "Entity MSAA Image"
		});

		app.outlineImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Attachment | TextureUsageBits::TextureUsageBits_Sampled,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R8_UNORM,
				.debugName = "Outline Selection Image"
		});

		app.viewportImage = gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Sampled,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.debugName = "Viewport Image"
		});
	}
	void CreateEditorMeshes(GX& gx, std::unordered_map<MeshPrimitiveType, MeshData>& defaults) {
		quadMeshData = gx.createMesh(Primitives::quadVertices, Primitives::quadIndices);
		planeMeshData = gx.createMesh(Primitives::planeVertices, Primitives::quadIndices);
		cubeMeshData = gx.createMesh(Primitives::cubeVertices, Primitives::cubeIndices);
		std::vector<Vertex> vertices = {};
		std::vector<uint32_t> indices = {};
		GenerateSphere(vertices, indices, 1.0f, 15, 15);
		sphereMeshData = gx.createMesh(vertices, indices);

		// fill in defaults
		defaults[MeshPrimitiveType::Quad] = quadMeshData;
		defaults[MeshPrimitiveType::Plane] = planeMeshData;
		defaults[MeshPrimitiveType::Cube] = cubeMeshData;
		defaults[MeshPrimitiveType::Sphere] = sphereMeshData;
	}
	void EditorApplication::_createVisualizerMeshes() {
		std::vector<Vertex> vertices;

		vertices.clear();
		GenerateArrow2DMesh(vertices, 2.f, 1.f, 0.5f, 0.8f);
		this->arrowmesh = this->getGX().createMesh(vertices);

		vertices.clear();
		GenerateSimpleSphere(vertices, 50);
		this->simplespheremesh = this->getGX().createMesh(vertices);

		vertices.clear();
		GenerateSpot(vertices, 10);
		this->spotmesh = this->getGX().createMesh(vertices);
	}
	void LoadEditorTextures(GX& gx) {
		// in scene
		lightbulbTexture.loadResource(Filesystem::GetRelativePath("textures/icons/lightbulb.png"));
		lightbulbTexture.assignHandle(gx.createTexture({
				.dimension = lightbulbTexture.getDimensions(),
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Sampled,
				.format = VK_FORMAT_R8G8B8A8_SRGB,
				.data = lightbulbTexture.getData(),
				.debugName = "Lightbulb Texture"
		}));
		sunTexture.loadResource(Filesystem::GetRelativePath("textures/icons/sun.png"));
		sunTexture.assignHandle(gx.createTexture({
				.dimension = sunTexture.getDimensions(),
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Sampled,
				.format = VK_FORMAT_R8G8B8A8_SRGB,
				.data = sunTexture.getData(),
				.debugName = "Sun Texture"
		}));
		spotlightTexture.loadResource(Filesystem::GetRelativePath("textures/icons/lamp-ceiling.png"));
		spotlightTexture.assignHandle(gx.createTexture({
				.dimension = spotlightTexture.getDimensions(),
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Sampled,
				.format = VK_FORMAT_R8G8B8A8_SRGB,
				.data = spotlightTexture.getData(),
				.debugName = "Spotlight Texture"
		}));
	}
	void LoadEditorShaders(GX& gx) {
		standardShader.loadResource(Filesystem::GetRelativePath("shaders/standard.slang"));
		standardShader.assignHandle(gx.createShader({
				.spirvBlob = standardShader.requestCode()
		}));
		primitiveShader.loadResource(Filesystem::GetRelativePath("shaders/EditorEXT/editor_primitives.slang"));
		primitiveShader.assignHandle(gx.createShader({
				.spirvBlob = primitiveShader.requestCode(),
				.pushConstantSize = primitiveShader.getPushSize()
		}));
		imageShader.loadResource(Filesystem::GetRelativePath("shaders/EditorEXT/editor_images.slang"));
		imageShader.assignHandle(gx.createShader({
				.spirvBlob = imageShader.requestCode(),
				.pushConstantSize = imageShader.getPushSize()
		}));
		solidShader.loadResource(Filesystem::GetRelativePath("shaders/EditorEXT/solid_shading.slang"));
		solidShader.assignHandle(gx.createShader({
				.spirvBlob = solidShader.requestCode(),
				.pushConstantSize = solidShader.getPushSize()
		}));
		infiniteGridShader.loadResource(Filesystem::GetRelativePath("shaders/EditorEXT/editor_grid.slang"));
		infiniteGridShader.assignHandle(gx.createShader({
				.spirvBlob = infiniteGridShader.requestCode(),
				.pushConstantSize = infiniteGridShader.getPushSize()
		}));
		fullscreenShader.loadResource(Filesystem::GetRelativePath("shaders/fullscreen_outline.slang"));
		fullscreenShader.assignHandle(gx.createShader({
				.spirvBlob = fullscreenShader.requestCode(),
				.pushConstantSize = fullscreenShader.getPushSize()
		}));
		pureMaskShader.loadResource(Filesystem::GetRelativePath("shaders/EditorEXT/mask.slang"));
		pureMaskShader.assignHandle(gx.createShader({
				.spirvBlob = pureMaskShader.requestCode(),
				.pushConstantSize = pureMaskShader.getPushSize()
		}));
	}

	void EditorApplication::onInitialize() {
		// Window Setup
		WindowSpec window_spec = {
				.videomode = VideoMode::Windowed,
				.title = "Slate Editor",
				.resizeable = true,
		};
		createWindow(window_spec);

		// Vulkan Setup
		VulkanInstanceInfo vk_info = {
				.app_name = "Slate Editor",
				.app_version = {0, 0, 1},
				.engine_name = "Slate Engine",
				.engine_version = {0, 0, 1}
		};
		GX& gx = this->_gx;
		gx.create(vk_info, getActiveWindow()->getGLFWWindow());
		GLTFLoader::_gx = &gx;

		NFD_Init();


		// editor essentials
		// hidden editor
		CreateEditorAttachments(gx, *this);
		CreateEditorMeshes(gx, defaultMeshPrimitiveTypes);
		this->_createVisualizerMeshes();
		// for edito viewport object selection
		_stagbuf = gx.createBuffer({
				.size = sizeof(uint32_t),
				.usage = BufferUsageBits::BufferUsageBits_Storage,
				.storage = StorageType::HostVisible,
				.debugName = "Viewport Selection Buffer"
		});
		// visible editor resources
		LoadEditorTextures(gx);
		LoadEditorShaders(gx);

		GOOGLE_PROTOBUF_VERIFY_VERSION;
		ClientSocket client;
		if (client.connect("/tmp/ipc.sock")) {
			SocketConnection connection = client.getConnection();

			engine::LoadSceneRequest sceneRequest;
//			sceneRequest.set_path("../nothing.json");
//			connection.fullProtoSend(sceneRequest);

		}


		// ImGui
		ImGuiRequiredData imgui_required = gx.requestImGuiRequiredData();
		this->InitImGui(imgui_required, getActiveWindow()->getGLFWWindow(), VK_FORMAT_R8G8B8A8_UNORM);
		{
			auto [ sampler, image_view ] = gx.requestViewportImageData(viewportImage);
			this->_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		{
			int w, h;
			void* data = stbi_load(Filesystem::GetRelativePath("textures/icons/file.png").c_str(), &w, &h, nullptr, 4);
			InternalTextureHandle fileIcon = gx.createTexture({
					.dimension = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)},
					.usage = TextureUsageBits::TextureUsageBits_Sampled,
					.format = VK_FORMAT_R8G8B8A8_SRGB,
					.data = data,
					.debugName = "File Icon"
			});
			this->_fileImageDS = ImGui_ImplVulkan_AddTexture(gx.getLinearSampler(), gx.getTextureImageView(fileIcon), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			stbi_image_free(data);
		}
		{
			int w, h;
			void *data = stbi_load(Filesystem::GetRelativePath("textures/icons/folder.png").c_str(), &w, &h, nullptr, 4);
			InternalTextureHandle folderIcon = gx.createTexture({
					.dimension = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)},
					.usage = TextureUsageBits::TextureUsageBits_Sampled,
					.format = VK_FORMAT_R8G8B8A8_SRGB,
					.data = data,
					.debugName = "Folder Icon"
			});
			this->_folderImageDS = ImGui_ImplVulkan_AddTexture(gx.getLinearSampler(), gx.getTextureImageView(folderIcon), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			stbi_image_free(data);
		}

		// do our themes setting, imgui fonts and style
		BuildStyle();

		const PipelineSpec::AttachmentFormats standardFormats = {
				.colorFormats = {
						gx.getTextureFormat(colorMSAAImage),
						gx.getTextureFormat(entityMSAAImage)
				},
				.depthFormat = gx.getTextureFormat(depthStencilMSAAImage)
		};
		const PipelineSpec::AttachmentFormats noidFormats = {
				.colorFormats = {
						gx.getTextureFormat(colorMSAAImage),
				},
				.depthFormat = gx.getTextureFormat(depthStencilMSAAImage)
		};
		const PipelineSpec::AttachmentFormats maskFormats = {
				.colorFormats = {
						gx.getTextureFormat(outlineImage)
				}
		};
		shadedModePipeline = gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::FILL,
				.blend = BlendingMode::OFF,
				.cull = CullMode::BACK,
				.multisample = SampleCount::X4,
				.formats = standardFormats,
				.shaderhandle = standardShader.getHandle()
		});
		wireframeModePipeline = gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::LINE,
				.blend = BlendingMode::OFF,
				.cull = CullMode::OFF,
				.multisample = SampleCount::X4,
				.formats = standardFormats,
				.shaderhandle = primitiveShader.getHandle()
		});
		unshadedModePipeline = gx.createPipeline({
			   .topology = TopologyMode::TRIANGLE,
			   .polygon = PolygonMode::FILL,
			   .blend = BlendingMode::OFF,
			   .cull = CullMode::BACK,
			   .multisample = SampleCount::X4,
			   .formats = standardFormats,
			   .shaderhandle = solidShader.getHandle()
	   });
		gridShaderPipeline = gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::FILL,
				.blend = BlendingMode::ALPHA_BLEND,
				.cull = CullMode::OFF,
				.multisample = SampleCount::X4,
				.formats = standardFormats,
				.shaderhandle = infiniteGridShader.getHandle()
		});

		wireframeVisualizerPipeline = gx.createPipeline({
				.topology = TopologyMode::STRIP,
				.polygon = PolygonMode::LINE,
				.blend = BlendingMode::OFF,
				.cull = CullMode::OFF,
				.multisample = SampleCount::X4,
				.formats = standardFormats,
				.shaderhandle = primitiveShader.getHandle()
		});
		filledVisualizerPipeline = gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::FILL,
				.blend = BlendingMode::ALPHA_BLEND,
				.multisample = SampleCount::X4,
				.formats = standardFormats,
				.shaderhandle = imageShader.getHandle(),
		});
		fullscreenPipeline = gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::FILL,
				.blend = BlendingMode::OFF,
				.multisample = SampleCount::X1,
				.formats = noidFormats,
				.shaderhandle = fullscreenShader.getHandle(),
		});
		pureOutlinePipeline = gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::FILL,
				.blend = BlendingMode::OFF,
				.multisample = SampleCount::X1,
				.formats = maskFormats,
				.shaderhandle = pureMaskShader.getHandle()
		});

		cubemapRes.loadResource(Filesystem::GetRelativePath("textures/hdri/qwantani_dusk_2_1k.hdr"));
		cubemapRes.assignHandle(gx.createTexture({
				.dimension = cubemapRes.getDimensions(),
				.usage = TextureUsageBits::TextureUsageBits_Sampled,
				.type = TextureType::Type_Cube,
				.format = VK_FORMAT_R32G32B32A32_SFLOAT,
				.data = cubemapRes.getData(),
				.debugName = "Cube Map Test"
		}));


		// init scene
		this->ctx.scene = new Scene();
		Scene& scene = *ctx.scene;



		MeshHandle fishhandle = _meshPool.create(Filesystem::GetRelativePath("models/BarramundiFish/glTF-Binary/BarramundiFish.glb"));
		MeshHandle suzannehandle = _meshPool.create(Filesystem::GetRelativePath("models/Suzanne/glTF/Suzanne.gltf"));
		ScriptHandle basichandle = _scriptPool.create(Filesystem::GetRelativePath("scripts/basic.lua"));

		GameEntity fish = scene.createEntity("Fishy");
		fish.addComponent<TransformComponent>().global.scale = glm::vec3{10};
		fish.addComponent<GeometryGLTFComponent>().handle = fishhandle;
		fish.addComponent<ScriptComponent>().handle = basichandle;

		GameEntity suzanne = scene.createEntity("Suzanne");
		suzanne.addComponent<TransformComponent>().global.position = glm::vec3{-5.f, 4.f, 0.f};
		suzanne.addComponent<GeometryGLTFComponent>().handle = suzannehandle;
		suzanne.addComponent<ScriptComponent>().handle = basichandle;

		GameEntity sphere = scene.createEntity("Cube");
		sphere.addComponent<TransformComponent>().global.position = glm::vec3{-2.f, 4.f, 0.f};
		sphere.addComponent<GeometryPrimitiveComponent>().mesh_type = MeshPrimitiveType::Cube;

		GameEntity quady = scene.createEntity("Quad");
		quady.addComponent<TransformComponent>().global.position = glm::vec3{1.f, 1.f, 1.f};
		quady.addComponent<GeometryPrimitiveComponent>().mesh_type = MeshPrimitiveType::Quad;

		GameEntity standardPlane = scene.createEntity("Standard Plane");
		standardPlane.addComponent<GeometryPrimitiveComponent>().mesh_type = MeshPrimitiveType::Plane;
		standardPlane.getComponent<TransformComponent>().global.scale = {10.f, 1.f, 10.f};

		sphere.addChild(quady);
		sphere.addChild(standardPlane);

		quady.addChild(fish);

		GameEntity defaultCube = scene.createEntity("Sphere");
		defaultCube.addComponent<TransformComponent>().global.position = glm::vec3{0.f, 5.f, 0.f};
		defaultCube.addComponent<GeometryPrimitiveComponent>().mesh_type = MeshPrimitiveType::Sphere;

		GameEntity pointlight1 = scene.createEntity("Point Light 1");
		pointlight1.addComponent<TransformComponent>().global.position = glm::vec3{2.f, 3.f, 2.f};
		pointlight1.addComponent<PointLightComponent>().point.Color = glm::vec3{1.f, 0.f, 0.f};

		GameEntity pointlight2 = scene.createEntity("Point Light 2");
		pointlight2.addComponent<TransformComponent>().global.position = glm::vec3{-3.f, 3.f, -3.f};
		pointlight2.addComponent<PointLightComponent>().point.Color = glm::vec3{0.2f, 0.3f, 0.8f};

		GameEntity pointlight3 = scene.createEntity("Point Light 3");
		pointlight3.addComponent<TransformComponent>().global.position = glm::vec3{-3.f, 0.3f, 3.f};
		pointlight3.addComponent<PointLightComponent>().point.Color = glm::vec3{0.212f, 0.949f, 0.129f};

		GameEntity environ = scene.createEntity("Environment");
		environ.addComponent<TransformComponent>().global.position = glm::vec3{-6.f, 6.f, -7.f};
		environ.addComponent<AmbientLightComponent>();

		GameEntity sunlight = scene.createEntity("Sun Light");
		sunlight.addComponent<TransformComponent>().global.position = glm::vec3{-7.f, 5.f, -4.f};
		sunlight.addComponent<DirectionalLightComponent>().directional.Color = glm::vec3{0.91, 0.882, 0.714};

		GameEntity spotlight = scene.createEntity("Spot Light 1");
		spotlight.addComponent<TransformComponent>().global.position = glm::vec3{2.f, 4.f, -4.f};
		spotlight.addComponent<SpotLightComponent>().spot.Color = glm::vec3{0.3f, 0.6f, 0.1f};

		ImGui_ImplGlfw_InstallCallbacks(_window.getGLFWWindow());
	}

	void EditorApplication::onTick() {
		if (GetInput().IsKeyJustClicked(KeyCode::Q)) {
			this->callStop();
		}
		for (GameEntity& entity : this->ctx.scene->GetAllEntitiesWithEXT<ScriptComponent>()) {
			ScriptComponent& component = entity.getComponent<ScriptComponent>();
		}
	}

	void EditorApplication::onSwapchainResize() {
		GX& gx = _gx;

		gx.destroy(colorResolveImage);
		gx.destroy(colorMSAAImage);
		gx.destroy(entityResolveImage);
		gx.destroy(entityMSAAImage);
		gx.destroy(depthStencilMSAAImage);
		gx.destroy(viewportImage);

		CreateEditorAttachments(gx, *this);
		// recreate descriptor set for the viewport image used by imgui
		auto [ sampler, image_view ] = gx.requestViewportImageData(viewportImage);
		this->_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void EditorApplication::onRender() {

		GX& gx = _gx;

		_camera.updateMatrices();
		GPU::CameraData cameraData = {
				.projectionMatrix = _camera.getProjectionMatrix(),
				.viewMatrix = _camera.getViewMatrix(),
				.position = _camera.getPosition()
		};

		AmbientLightComponent env = this->ctx.scene->GetEnvironment();
		GPU::LightingData lightingData{
				.ambient{
						.Color = env.ambient.Color,
						.Intensity = env.ambient.Intensity}};
		lightingData.ClearDynamics();

		for (const GameEntity entity : this->ctx.scene->GetAllEntitiesWithEXT<DirectionalLightComponent>()) {
			const TransformComponent entity_transform = entity.getComponent<TransformComponent>();
			const DirectionalLightComponent entity_light = entity.getComponent<DirectionalLightComponent>();

			lightingData.directional.Direction = entity_transform.global.rotation * glm::vec3(0, -1, 0);
			// stuff from properties panel
			lightingData.directional.Color = entity_light.directional.Color;
			lightingData.directional.Intensity = entity_light.directional.Intensity;
		}
		int i = 0;
		for (const GameEntity entity : this->ctx.scene->GetAllEntitiesWithEXT<PointLightComponent>()) {
			const TransformComponent entity_transform = entity.getComponent<TransformComponent>();
			const PointLightComponent entity_light = entity.getComponent<PointLightComponent>();

			lightingData.points[i].Position = entity_transform.global.position;
			// stuff from properties panel
			lightingData.points[i].Color = entity_light.point.Color;
			lightingData.points[i].Intensity = entity_light.point.Intensity;
			lightingData.points[i].Range = entity_light.point.Range;
			i++;
		}
		i = 0;
		for (const GameEntity entity : this->ctx.scene->GetAllEntitiesWithEXT<SpotLightComponent>()) {
			const TransformComponent entity_transform = entity.getComponent<TransformComponent>();
			const SpotLightComponent entity_light = entity.getComponent<SpotLightComponent>();

			lightingData.spots[i].Position = entity_transform.global.position;
			lightingData.spots[i].Direction = entity_transform.global.rotation * glm::vec3(0, -1, 0);
			// stuff from properties panel
			lightingData.spots[i].Color = entity_light.spot.Color;
			lightingData.spots[i].Intensity = entity_light.spot.Intensity;
			lightingData.spots[i].Size = entity_light.spot.Size;
			lightingData.spots[i].Blend = entity_light.spot.Blend;
			i++;
		}



		ctx.scene->Tick(getTime().getDeltaTime());
		this->_guiUpdate();
		{
			CommandBuffer& cmd = gx.acquireCommand();
			{
				cmd.cmdTransitionLayout(colorMSAAImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				cmd.cmdTransitionLayout(depthStencilMSAAImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				cmd.cmdTransitionLayout(entityResolveImage, VK_IMAGE_LAYOUT_GENERAL);
				cmd.cmdTransitionLayout(entityMSAAImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

				// you must update buffer before using it in push constants
				const GPU::PerFrameData perframedata = {
						.camera = cameraData,
						.lighting = lightingData,
						.time = (float)getTime().getElapsedTime(),
						.resolution = { gx.getSwapchainExtent().width, gx.getSwapchainExtent().height }
				};
				cmd.cmdUpdateBuffer(gx._globalBufferHandle, perframedata);

				RenderPass first = {
						.color = {
								RenderPass::ColorAttachmentDesc{
										.texture = colorMSAAImage,
										.loadOp = LoadOperation::CLEAR,
										.storeOp = StoreOperation::STORE,
										.clear = gx._clearColor
								},
								RenderPass::ColorAttachmentDesc{
										.texture = entityMSAAImage,
										.loadOp = LoadOperation::CLEAR,
										.storeOp = StoreOperation::STORE,
										.clear = RGBA{-1, 0, 0, 0}
								}
						},
						.depth = {
								.texture = depthStencilMSAAImage,
								.loadOp = LoadOperation::CLEAR,
								.storeOp = StoreOperation::STORE,
								.clear = 1.f
						}
				};
				RenderPass intermediate = {
						.color = {
								RenderPass::ColorAttachmentDesc{
										.texture = colorMSAAImage,
										.loadOp = LoadOperation::LOAD,
										.storeOp = StoreOperation::STORE,
								},
								RenderPass::ColorAttachmentDesc{
										.texture = entityMSAAImage,
										.loadOp = LoadOperation::LOAD,
										.storeOp = StoreOperation::STORE,
								}
						},
						.depth = {
								.texture = depthStencilMSAAImage,
								.loadOp = LoadOperation::LOAD,
								.storeOp = StoreOperation::STORE,
						}
				};
				RenderPass last = {
						.color = {
								RenderPass::ColorAttachmentDesc{
										.texture = colorMSAAImage,
										.resolveTexture = colorResolveImage,
										.loadOp = LoadOperation::LOAD,
										.storeOp = StoreOperation::STORE,
								},
								RenderPass::ColorAttachmentDesc{
										.texture = entityMSAAImage,
										.resolveTexture = entityResolveImage,
										.resolveMode = ResolveMode::SAMPLE_ZERO,
										.loadOp = LoadOperation::LOAD,
										.storeOp = StoreOperation::STORE,
								}
						},
						.depth = {
								.texture = depthStencilMSAAImage,
								.loadOp = LoadOperation::LOAD,
								.storeOp = StoreOperation::NO_CARE,
						}
				};
				// gui pass needs store op to be true!
				RenderPass fullscreenPass = {
						.color = {
								RenderPass::ColorAttachmentDesc{
										.texture = colorResolveImage,
										.loadOp = LoadOperation::LOAD,
										.storeOp = StoreOperation::STORE
								}
						}
				};
				RenderPass outlinePass = {
						.color = {
								RenderPass::ColorAttachmentDesc{
										.texture = outlineImage,
										.loadOp = LoadOperation::NO_CARE,
										.storeOp = StoreOperation::STORE,
										.clear = RGBA{0, 0, 0, 1}
								}
						}
				};
				RenderPass guiPass = {
						.color = {
								RenderPass::ColorAttachmentDesc{
										.texture = colorResolveImage,
										.loadOp = LoadOperation::NO_CARE,
										.storeOp = StoreOperation::STORE
								}
						}
				};

				// same texture that must be submitted at the end of cmd buffer
				InternalTextureHandle swapchainTexture = gx.acquireCurrentSwapchainTexture();

				cmd.cmdBeginRendering(first);
				if (_viewportMode == ViewportModes::SHADED || _viewportMode == ViewportModes::SOLID_WIREFRAME) {

					// for shaded
					cmd.cmdBindRenderPipeline(shadedModePipeline);
					cmd.cmdBindDepthState({
							.compareOp = CompareOperation::CompareOp_Less,
							.isDepthWriteEnabled = true,
					});
					for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<GeometryPrimitiveComponent>()) {
						const MeshPrimitiveType type = entity.getComponent<GeometryPrimitiveComponent>().mesh_type;
						if (type == MeshPrimitiveType::Empty) continue;
						const MeshData &mesh = defaultMeshPrimitiveTypes[type];

						GPU::PerObjectData constants = {
								.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
								.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
								.id = (uint32_t) entity.getHandle(),
						};
						cmd.cmdPushConstants(constants);
						cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
						cmd.cmdDrawIndexed(mesh.getIndexCount());
					}
					for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<GeometryGLTFComponent>()) {
						const auto meshSource = _meshPool.get(entity.getComponent<GeometryGLTFComponent>().handle);
						for (int k = 0; k < meshSource->getMeshCount(); k++) {
							const MeshData& mesh = meshSource->getBuffers()[k];
							GPU::PerObjectData constants = {
									.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
									.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
									.id = (uint32_t) entity.getHandle(),
							};
							cmd.cmdPushConstants(constants);
							cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
							cmd.cmdDrawIndexed(mesh.getIndexCount());
						}
					}
				}
				if (_viewportMode == ViewportModes::SOLID_WIREFRAME || _viewportMode == ViewportModes::WIREFRAME) {
					cmd.cmdBindRenderPipeline(wireframeModePipeline);
					cmd.cmdBindDepthState({
							.compareOp = CompareOperation::CompareOp_Less,
							.isDepthWriteEnabled = true,
					});
					if (_viewportMode == ViewportModes::SOLID_WIREFRAME) {
						cmd.cmdSetDepthBiasEnable(true);
						cmd.cmdSetDepthBias(0.f, -1.f, 0.f);
					}

					for (const GameEntity& entity: ctx.scene->GetAllEntitiesWithEXT<GeometryPrimitiveComponent>()) {
						const MeshPrimitiveType type = entity.getComponent<GeometryPrimitiveComponent>().mesh_type;
						if (type == MeshPrimitiveType::Empty) continue;
						const MeshData &mesh = defaultMeshPrimitiveTypes[type];

						GPU::PushConstants_EditorPrimitives constants = {
								.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
								.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
								.color = {1, 0, 0}// we just keep red for now
						};
						cmd.cmdPushConstants(constants);
						cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
						cmd.cmdDrawIndexed(mesh.getIndexCount());
					}
					for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<GeometryGLTFComponent>()) {
						const auto meshSource = _meshPool.get(entity.getComponent<GeometryGLTFComponent>().handle);
						for (int k = 0; k < meshSource->getMeshCount(); k++) {
							const MeshData& mesh = meshSource->getBuffers()[k];
							GPU::PushConstants_EditorPrimitives constants = {
									.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
									.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
									.color = {1, 0, 0}// we just keep red for now
							};
							cmd.cmdPushConstants(constants);
							cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
							cmd.cmdDrawIndexed(mesh.getIndexCount());
						}
					}
				}
				if (_viewportMode == ViewportModes::UNSHADED) {
					cmd.cmdBindRenderPipeline(unshadedModePipeline);
					cmd.cmdBindDepthState({
							.compareOp = CompareOperation::CompareOp_Less,
							.isDepthWriteEnabled = true,
					});
					for (const GameEntity& entity: ctx.scene->GetAllEntitiesWithEXT<GeometryPrimitiveComponent>()) {
						const MeshPrimitiveType type = entity.getComponent<GeometryPrimitiveComponent>().mesh_type;
						if (type == MeshPrimitiveType::Empty) continue;
						const MeshData &mesh = defaultMeshPrimitiveTypes[type];

						GPU::PushConstants_EditorSolidShading constants = {
								.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
								.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
								.color = glm::vec3{0.4f}, // gray
								.id = (uint32_t) entity.getHandle()
						};
						cmd.cmdPushConstants(constants);
						cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
						cmd.cmdDrawIndexed(mesh.getIndexCount());
					}
					for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<GeometryGLTFComponent>()) {
						const auto meshSource = _meshPool.get(entity.getComponent<GeometryGLTFComponent>().handle);
						for (int k = 0; k < meshSource->getMeshCount(); k++) {
							const MeshData& mesh = meshSource->getBuffers()[k];
							GPU::PushConstants_EditorSolidShading constants = {
									.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
									.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
									.color = glm::vec3{0.4f}, // gray
									.id = (uint32_t) entity.getHandle()
							};
							cmd.cmdPushConstants(constants);
							cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
							cmd.cmdDrawIndexed(mesh.getIndexCount());
						}
					}
				}
				cmd.cmdEndRendering();

				if (_gridEnabled) {
					cmd.cmdBeginRendering(intermediate);
					{
						cmd.cmdBindRenderPipeline(gridShaderPipeline);
						cmd.cmdBindDepthState({
								.compareOp = CompareOperation::CompareOp_Less,
								.isDepthWriteEnabled = true,
						});
						cmd.cmdSetDepthBiasEnable(true);
						cmd.cmdSetDepthBias(0.f, -1.f, 0.f);
						cmd.cmdDraw(6);
					}
					cmd.cmdEndRendering();
					cmd.cmdBeginRendering(intermediate);
					{
						// WIREFRAME EDITOR RENDERING //
						{
							cmd.cmdBindRenderPipeline(wireframeVisualizerPipeline);
							cmd.cmdBindDepthState({
									.compareOp = CompareOperation::CompareOp_Less,
									.isDepthWriteEnabled = true,
							});
							// POINT LIGHTS
							for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<PointLightComponent>()) {
								glm::mat4 model = TransformToModelMatrix(entity.getComponent<TransformComponent>(), false, false);
								glm::mat4 spheremodel = glm::scale(model, glm::vec3(entity.getComponent<PointLightComponent>().point.Range));

								GPU::PushConstants_EditorPrimitives constants = {
										.modelMatrix = spheremodel,
										.vertexBufferAddress = gx.gpuAddress(simplespheremesh.getVertexBufferHandle()),
								};
								cmd.cmdPushConstants(constants);
								cmd.cmdDraw(simplespheremesh.getVertexCount());
							}
							// SPOT LIGHTS
							for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<SpotLightComponent>()) {
								glm::mat4 model = TransformToModelMatrix(entity.getComponent<TransformComponent>(), false);
								float size = entity.getComponent<SpotLightComponent>().spot.Size;
								float halfAngle = glm::radians(size * 0.5f);
								float radius = sin(halfAngle) / sin(glm::radians(5.0f));
								float height = radius / tan(halfAngle);
								glm::mat4 spotmodel = glm::scale(model, glm::vec3{radius, height, radius});

								GPU::PushConstants_EditorPrimitives constants = {
										.modelMatrix = spotmodel,
										.vertexBufferAddress = gx.gpuAddress(spotmesh.getVertexBufferHandle()),
								};
								cmd.cmdPushConstants(constants);
								cmd.cmdDraw(spotmesh.getVertexCount());
							}
							// DIRECTIONAL LIGHTS
							for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<DirectionalLightComponent>()) {
								glm::mat4 model = TransformToModelMatrix(entity.getComponent<TransformComponent>(), false);
								GPU::PushConstants_EditorPrimitives constants = {
										.modelMatrix = model,
										.vertexBufferAddress = gx.gpuAddress(arrowmesh.getVertexBufferHandle()),
								};
								cmd.cmdPushConstants(constants);
								cmd.cmdDraw(arrowmesh.getVertexCount());
							}
						}
					}
					cmd.cmdEndRendering();

					cmd.cmdBeginRendering(last);
					{
						// FILLED EDITOR RENDERING //
						{
							cmd.cmdBindRenderPipeline(filledVisualizerPipeline);
							cmd.cmdBindDepthState({
									.compareOp = CompareOperation::CompareOp_Less,
									.isDepthWriteEnabled = true,
							});
							const MeshData &quad_mesh_ref = defaultMeshPrimitiveTypes[MeshPrimitiveType::Quad];
							cmd.cmdBindIndexBuffer(quad_mesh_ref.getIndexBufferHandle());// because we just use the same quad mesh, we can bind once at the beginning
							// POINT LIGHTS
							for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<PointLightComponent>()) {
								glm::mat4 model = TransformToModelMatrix(entity.getComponent<TransformComponent>(), false, false);
								model = BillboardModelMatrix(model, _camera);
								model = glm::scale(model, glm::vec3{0.5});

								GPU::PushConstants_EditorImages constants = {
										.modelMatrix = model,
										.color = entity.getComponent<PointLightComponent>().point.Color,
										.vertexBufferAddress = gx.gpuAddress(quad_mesh_ref.getVertexBufferHandle()),
										.id = (uint32_t) entity.getHandle(),
										.textureId = lightbulbTexture.getHandle().index()};
								cmd.cmdPushConstants(constants);
								cmd.cmdDrawIndexed(quad_mesh_ref.getIndexCount());
							}
							// SPOT LIGHTS
							for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<SpotLightComponent>()) {
								glm::mat4 model = TransformToModelMatrix(entity.getComponent<TransformComponent>(), false, false);
								model = BillboardModelMatrix(model, _camera);
								model = glm::scale(model, glm::vec3{0.5});

								GPU::PushConstants_EditorImages constants = {
										.modelMatrix = model,
										.color = entity.getComponent<SpotLightComponent>().spot.Color,
										.vertexBufferAddress = gx.gpuAddress(quad_mesh_ref.getVertexBufferHandle()),
										.id = (uint32_t) entity.getHandle(),
										.textureId = spotlightTexture.getHandle().index()};
								cmd.cmdPushConstants(constants);
								cmd.cmdDrawIndexed(quad_mesh_ref.getIndexCount());
							}
							// DIRECTIONAL LIGHTS
							for (const GameEntity &entity: ctx.scene->GetAllEntitiesWithEXT<DirectionalLightComponent>()) {
								glm::mat4 model = TransformToModelMatrix(entity.getComponent<TransformComponent>(), false, false);
								model = BillboardModelMatrix(model, _camera);
								model = glm::scale(model, glm::vec3{0.5});

								GPU::PushConstants_EditorImages constants = {
										.modelMatrix = model,
										.color = entity.getComponent<DirectionalLightComponent>().directional.Color,
										.vertexBufferAddress = gx.gpuAddress(quad_mesh_ref.getVertexBufferHandle()),
										.id = (uint32_t) entity.getHandle(),
										.textureId = sunTexture.getHandle().index()};
								cmd.cmdPushConstants(constants);
								cmd.cmdDrawIndexed(quad_mesh_ref.getIndexCount());
							}
						}
					}
					cmd.cmdEndRendering();
				} else {
					cmd.cmdBeginRendering(last);
					cmd.cmdEndRendering();
				}

				// render to outline image to be sampled in the next step
				cmd.cmdTransitionLayout(outlineImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				cmd.cmdBeginRendering(outlinePass);
				if (ctx.activeEntity.has_value()) {
					const GameEntity activeEntity = ctx.activeEntity.value();
					cmd.cmdBindRenderPipeline(pureOutlinePipeline);
					{
						if (activeEntity.hasComponent<GeometryPrimitiveComponent>()) {
							const MeshPrimitiveType type = activeEntity.getComponent<GeometryPrimitiveComponent>().mesh_type;
							if (type != MeshPrimitiveType::Empty) {
								const MeshData& mesh = defaultMeshPrimitiveTypes[type];
								struct P {
									glm::mat4 modelMatrix;
									VkDeviceAddress vertexBufferAddress;
								} constants {
										.modelMatrix = TransformToModelMatrix(activeEntity.getComponent<TransformComponent>()),
										.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
								};
								cmd.cmdPushConstants(constants);
								cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
								cmd.cmdDrawIndexed(mesh.getIndexCount());
							}
						}
						if (activeEntity.hasComponent<GeometryGLTFComponent>()) {
							const auto meshSource = _meshPool.get(activeEntity.getComponent<GeometryGLTFComponent>().handle);
							for (int k = 0; k < meshSource->getMeshCount(); k++) {
								const MeshData& mesh = meshSource->getBuffers()[k];
								struct P {
									glm::mat4 modelMatrix;
									VkDeviceAddress vertexBufferAddress;
								} constants {
										.modelMatrix = TransformToModelMatrix(activeEntity.getComponent<TransformComponent>()),
										.vertexBufferAddress = gx.gpuAddress(mesh.getVertexBufferHandle()),
								};
								cmd.cmdPushConstants(constants);
								cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
								cmd.cmdDrawIndexed(mesh.getIndexCount());
							}
						}
					}
				}
				cmd.cmdEndRendering();
				cmd.cmdTransitionLayout(outlineImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				cmd.cmdBlitImage(colorResolveImage, viewportImage);
				cmd.cmdTransitionLayout(colorResolveImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				cmd.cmdTransitionLayout(viewportImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


				// fullscreen shader test
				cmd.cmdBeginRendering(fullscreenPass);
				{
					cmd.cmdBindRenderPipeline(fullscreenPipeline);
					struct P {
						uint32_t imageTexId;
						uint32_t maskTexId;
					} push {
							.imageTexId = viewportImage.index(),
							.maskTexId = outlineImage.index()
					};
					cmd.cmdPushConstants(push);
					cmd.cmdDraw(3);
				}
				cmd.cmdEndRendering();


				cmd.cmdBlitImage(colorResolveImage, viewportImage);
				cmd.cmdTransitionLayout(viewportImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				cmd.cmdTransitionLayout(colorResolveImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


				// render ui
				{
					cmd.cmdBeginRendering(guiPass);
					ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.requestVkCmdBuffer());
					cmd.cmdEndRendering();
				}
				// close out
				cmd.cmdTransitionLayout(colorResolveImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				cmd.cmdTransitionSwapchainLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				cmd.cmdBlitToSwapchain(colorResolveImage);
				cmd.cmdTransitionSwapchainLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

				gx.submitCommand(cmd, swapchainTexture);
			}
		}
	}
	void EditorApplication::onShutdown() {
		GX& gx = getGX();


		gx.destroy(this->colorResolveImage);
		gx.destroy(this->colorMSAAImage);
		gx.destroy(this->depthStencilMSAAImage);
		gx.destroy(this->entityMSAAImage);
		gx.destroy(this->entityResolveImage);
		gx.destroy(this->viewportImage);
		gx.destroy(this->_stagbuf);

		gx.destroy(standardShader.getHandle());
		gx.destroy(primitiveShader.getHandle());
		gx.destroy(imageShader.getHandle());

		gx.destroy(shadedModePipeline);
		gx.destroy(wireframeVisualizerPipeline);
		gx.destroy(filledVisualizerPipeline);


		gx.destroy(quadMeshData.getVertexBufferHandle());
		gx.destroy(quadMeshData.getIndexBufferHandle());
		gx.destroy(planeMeshData.getVertexBufferHandle());
		gx.destroy(planeMeshData.getIndexBufferHandle());
		gx.destroy(cubeMeshData.getVertexBufferHandle());
		gx.destroy(cubeMeshData.getIndexBufferHandle());
		gx.destroy(sphereMeshData.getVertexBufferHandle());
		gx.destroy(sphereMeshData.getIndexBufferHandle());

		gx.destroy(arrowmesh.getVertexBufferHandle());
		gx.destroy(simplespheremesh.getVertexBufferHandle());
		gx.destroy(spotmesh.getVertexBufferHandle());

		gx.destroy(lightbulbTexture.getHandle());
		gx.destroy(spotlightTexture.getHandle());
		gx.destroy(sunTexture.getHandle());

		NFD_Quit();
		// got to call this prior to imgui shutdown
		Application::getGX().deviceWaitIdle();
		// this order specifically
		ImGui_ImplVulkan_RemoveTexture(_viewportImageDescriptorSet);
		ImGui_ImplVulkan_Shutdown();
//		vkDestroyDescriptorPool(gx._backend.getDevice(), _imguiDescriptorPool, nullptr);
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorApplication::_settingsUpdate() {

	}
	void EditorApplication::_guiUpdate() {
		// required imgui opening
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		// below is all dockspace setup
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton; // flags for our Dockspace, which will be the whole screen

		// set the parent window's position, m_Count, and viewport to match that of the main viewport. This is so the parent window
		// completely covers the main viewport, giving it a "full-screen" feel.
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		// entire window flags
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		// make manipulation inaccessible to the user (no titlebar, resize/move, or navigation)
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// set the parent window's styles to match that of the main viewport:
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);             // No corner rounding on the window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);           // No border around the window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));// remove window padding for "fullscreen" effect

		ImGui::Begin("DockSpace Main", nullptr, window_flags);
		ImGui::PopStyleVar(3);// pop previous styles, (padding, rounding, border size)
		// The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		{
			// everything thats not a dedicated panel
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
					if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
					if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
					if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) {}
					ImGui::Separator();
					if (ImGui::MenuItem("Settings")) {
//						createWindow({
//								.videomode = VideoMode::Windowed,
//								.title = "Slate Editor Settings",
//								.resizeable = false,
//								.width = 750,
//								.height = 350,
//						});
//						_settingsUpdate();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Exit", "esc")) {
						this->callStop();
					};
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit")) {
					if (ImGui::MenuItem("BLOW UP", "Ctrl+B")) {}
					ImGui::EndMenu();
				}
			}
			ImGui::EndMainMenuBar();

			// everything that is a panel
			{
				_onViewportPanelUpdate();
				_onScenePanelUpdate();
				_onPropertiesPanelUpdate();
				_onAssetPanelUpdate();
			}
			// debug info
			{
				ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
				ImGui::SetNextWindowBgAlpha(0.5f);
				ImGui::Begin("Info - Debug", nullptr, windowFlags);
				ImGui::Text("Frame: %u", getGX().getFrameNum());
				ImGuiIO& io = ImGui::GetIO();
				ImGui::Text("Platform Name: %s", io.BackendPlatformName ? io.BackendPlatformName : "Unknown Platform");
				ImGui::Text("Backend RenderManager: %s", io.BackendRendererName ? io.BackendRendererName : "Unknown RenderManager");
				ImGui::Text("Capturing Mouse: %s", io.WantCaptureMouse ? "True" : "False");
				ImGui::Text("Display Size: %.1f x %.1f", io.DisplaySize.x, io.DisplaySize.y);

				ImGui::Text("Swapchain Size: %.1u x %.1u", getGX().getSwapchainExtent().width, getGX().getSwapchainExtent().height);
				ImGui::Text("Image Viewport Size: %.1u x %.1u", getGX().getTextureExtent(colorMSAAImage).width, getGX().getTextureExtent(colorMSAAImage).height);

				ImGui::Text("Display Framebuffer Scale: %.1f x %.1f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
				ImGui::Text("ImGui Framerate: %.2f", io.Framerate);
				// should be the same if everything is correct
				ImGui::Text("Slate Delta Time: %.2f", this->getTime().getDeltaTime());
				ImGui::Text("ImGui Delta Time: %.2f", io.DeltaTime);
//				if (this->ctx.hoveredEntity.has_value()) {
//					GameEntity hovered_entity = this->ctx.hoveredEntity.value();
//					ImGui::Text("Hovered Entity: %s | %u", hovered_entity.getName().c_str(), static_cast<int>(hovered_entity.getHandle()));
//				}
//				else ImGui::Text("Hovered Entity: 'NONE'");
				ImGui::End();
			}
			ImGui::End();// last end statement, dont put more imgui calls after this
		}
		// required imgui ending
		{
			ImGui::Render();
		}
	}
}