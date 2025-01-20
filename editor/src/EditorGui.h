//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once

#include <Slate/Window.h>
#include <Slate/Renderer.h>

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "ViewportCamera.h"
namespace Slate {
	class EditorGui {
	public:
		void InjectWindow(const Ref<Window>& windowRef) { _windowRef = windowRef; }
		void InjectViewportCamera(const Ref<ViewportCamera>& cameraRef) { _cameraRef = cameraRef; }
		void OnAttach(VulkanEngine& engine);
		void OnDetach();

		void OnUpdate(InputSystem& inputSystem, VulkanEngine &engine);
		void Render(VkCommandBuffer cmd, VkImageView imageView, VkExtent2D extent2D);
	private:
		bool _isCameraControlActive = false;
		Ref<ViewportCamera> _cameraRef;
		Ref<Window> _windowRef;
		ImTextureID sceneTexture;
	private: // all of the panels
		void OnPropertiesPanelUpdate();
		void OnViewportPanelUpdate(InputSystem& inputSystem);
		void OnScenePanelUpdate();
		void OnAssetPanelUpdate();
	};
}