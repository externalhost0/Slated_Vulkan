//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once

#include <Slate/Window.h>
#include <Slate/Renderer.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include "ViewportCamera.h"
#include "Context.h"

namespace Slate {
	enum struct ViewportModes {
		SHADED,
		UNSHADED,
		WIREFRAME,
		SOLID_WIREFRAME
	};
	enum struct HoverWindow {
		ViewportWindow,
		PropertiesWindow,
		ScenePanel,
		AssetsPanel
	};
	class EditorGui {
	public:
		void OnAttach(GLFWwindow* pNativeWindow);
		void OnDetach() const;

		void OnUpdate();
		void Render() const;
		bool gridIsEnabled = true;

		glm::vec2 _viewportBounds[2] {};

		bool IsMouseInViewportBounds();

		bool _isCameraControlActive = false;
	public: // imgui vk resources
		ImTextureID sceneTexture{};
		ViewportModes _viewportMode = ViewportModes::SHADED;
		HoverWindow _currenthovered;



		VulkanEngine* engine = nullptr;
		Context* pActiveContext = nullptr;
		ViewportCamera* pCamera = nullptr;

		ImGuizmo::MODE _guizmoSpace = ImGuizmo::MODE::WORLD;
		ImGuizmo::OPERATION _guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	private:
		std::optional<Entity> hoveredEntity;

	private:
		glm::vec2 _viewportSize {};

	private:
		vktypes::AllocatedBuffer stagbuf;

	private: // all of the panels
		void OnViewportAttach();
		void OnPropertiesPanelUpdate();
		void OnViewportPanelUpdate();
		void OnScenePanelUpdate();
		void OnAssetPanelUpdate();
	};
}