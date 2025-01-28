//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once

#include <Slate/Window.h>
#include <Slate/Renderer.h>


#include <imgui_impl_vulkan.h>
#include <ImGuizmo.h>
#include <vulkan/vulkan_core.h>

#include "ViewportCamera.h"
#include "Context.h"

namespace Slate {
	class EditorGui {
	public:
		void OnAttach(GLFWwindow* pNativeWindow);
		void OnDetach(VkDevice& device) const;

		void OnUpdate();
		void Render(VkCommandBuffer cmd, VkImageView imageView, VkExtent2D extent2D);
	private:
		bool _isCameraControlActive = false;
	public: // imgui vk resources
		VkSampler sampler{};
		ImTextureID sceneTexture{};



		VulkanEngine* engine = nullptr;
		Context* pActiveContext = nullptr;
		ViewportCamera* pCamera = nullptr;
	private:
		Entity hoveredEntity {};
		ImGuizmo::MODE _guizmoSpace = ImGuizmo::MODE::WORLD;
		ImGuizmo::OPERATION _guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	private:
		bool IsMouseInViewportBounds();
		glm::vec2 _viewportBounds[2] {};
		glm::vec2 _viewportSize {};

	private:
		bool gridIsEnabled = true;
		vktypes::MeshData gridmesh;

	private: // all of the panels
		void OnViewportAttach();
		void OnPropertiesPanelUpdate();
		void OnViewportPanelUpdate();
		void OnScenePanelUpdate();
		void OnAssetPanelUpdate();
	};
}