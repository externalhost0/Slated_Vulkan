//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <Slate/IApplication.h>

#include <Slate/ECS/Entity.h>
#include <Slate/ECS/Scene.h>
#include <Slate/SmartPointers.h>
#include <Slate/Window.h>

#include "ViewportCamera.h"

#include "imgui/imgui.h"
#include "ImGuizmo.h"

namespace Slate {
	enum struct ViewportModes : char {
		SHADED,
		UNSHADED,
		WIREFRAME,
		SOLID_WIREFRAME
	};
	enum struct HoverWindow : char {
		ViewportWindow,
		PropertiesWindow,
		ScenePanel,
		AssetsPanel
	};

	struct Context
	{
		Optional<GameEntity> activeEntity;
		Optional<GameEntity> hoveredEntity;
		Scene* scene;
	};

	class EditorApplication : public IApplication {
	protected:
		// called
		void onInitialize() override;
		void onTick() override;
		void onRender() override;
		void onShutdown() override;

		// callback
		void onWindowMinimize() override;
		void onSwapchainResize() override;

	private:
		void _guiUpdate();
		void _displayEntityNode(GameEntity entity);
		void _onViewportPanelUpdate();
		void _onScenePanelUpdate();
		void _onPropertiesPanelUpdate();
		void _onAssetPanelUpdate();

		void _createVisualizerMeshes();

		bool IsMouseInViewportBounds();
		void InitImGui(ImGuiRequiredData req, GLFWwindow* glfwWindow, VkFormat format);
	public:
		TextureHandle colorResolveImage;
		TextureHandle colorMSAAImage;
		TextureHandle depthStencilMSAAImage;
		TextureHandle entityResolveImage;
		TextureHandle entityMSAAImage;
		TextureHandle viewportImage;

		BufferHandle _stagbuf;


		MeshData arrowmesh;
		MeshData simplespheremesh;
		MeshData spotmesh;
	private:
		Context ctx;

		std::unordered_map<MeshPrimitiveType, MeshData> defaultMeshPrimitiveTypes;

		HoverWindow _currenthovered;

		ViewportCamera _camera;
		ViewportModes _viewportMode = ViewportModes::SHADED;
		ImGuizmo::MODE _guizmoSpace = ImGuizmo::MODE::WORLD;
		ImGuizmo::OPERATION _guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		bool _isCameraControlActive = false;
		bool _gridEnabled = true;
		glm::vec2 _viewportBounds[2]{};
		glm::vec2 _viewportSize{};


		VkDescriptorSet _viewportImageDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorPool _imguiDescriptorPool = VK_NULL_HANDLE;
	};
}
