//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once

#include <Slate/Window.h>
#include <Slate/Renderer.h>


#include <vulkan/vulkan.h>
#include <imgui_impl_vulkan.h>
#include <ImGuizmo.h>

#include "ViewportCamera.h"
#include "Context.h"

namespace Slate {
	class EditorGui {
	public:
		void OnAttach(GLFWwindow* pNativeWindow);
		void OnDetach() const;

		void OnUpdate();
		void Render() const;
		bool gridIsEnabled = true;

		vktypes::MeshData gridmesh;

		glm::vec2 _viewportBounds[2] {};

		bool IsMouseInViewportBounds();

	private:
		bool _isCameraControlActive = false;
	public: // imgui vk resources
		ImTextureID sceneTexture{};



		VulkanEngine* engine = nullptr;
		Context* pActiveContext = nullptr;
		ViewportCamera* pCamera = nullptr;
	private:
		std::optional<Entity> hoveredEntity;

		ImGuizmo::MODE _guizmoSpace = ImGuizmo::MODE::WORLD;
		ImGuizmo::OPERATION _guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
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