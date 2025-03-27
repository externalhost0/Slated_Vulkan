//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <Slate/SmartPointers.h>
#include <Slate/ShaderPass.h>
#include <Slate/Entity.h>
#include <Slate/Scene.h>
#include <Slate/Window.h>

#include "Slate/RenderEngine.h"
#include "ViewportCamera.h"

#include "imgui/imgui.h"
#include "ImGuizmo.h"

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

	class Editor {
	public:
		inline void Run() {
			this->Initialize();
			while (this->continueloop) {
				this->Loop();
			}
			this->Shutdown();
		}

		void OnKey(int key, int action, int mods);
		void OnMouseButton();
		void OnMouseMove();
		void OnWindowResize(int width, int height);
	private:
		void Initialize();
		void Loop();
		void Shutdown();

		// just for organization
		void Render();
		void RenderVisualizers();
		void RenderReal();
		void GuiUpdate();

		void OnPropertiesPanelUpdate();
		void OnViewportPanelUpdate();
		void OnScenePanelUpdate();
		void OnAssetPanelUpdate();

		void CreateEditorImages();
		bool IsMouseInViewportBounds();
	private:
		bool continueloop = true;
		bool gridEnabled = true;

		double lastTime = 0.f;
		double deltaTime = 0.f;

		ViewportModes _viewportMode = ViewportModes::SHADED;
		HoverWindow _currenthovered;
		ImGuizmo::MODE _guizmoSpace = ImGuizmo::MODE::WORLD;
		ImGuizmo::OPERATION _guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
		bool _isCameraControlActive = false;
		glm::vec2 _viewportBounds[2] {};
		ImTextureID sceneTexture{};
		glm::vec2 _viewportSize {};
		vktypes::AllocatedBuffer stagbuf;

		Optional<Shared<Entity>> activeEntity;
		Optional<Shared<Entity>> hoveredEntity;

		Window mainWindow;
		RenderEngine engine;
		ViewportCamera camera;
		Shared<Scene> scene;

		vktypes::AllocatedImage _colorResolveImage;
		vktypes::AllocatedImage _colorMSAAImage;
		vktypes::AllocatedImage _depthStencilMSAAImage;
		vktypes::AllocatedImage _entityImage;
		vktypes::AllocatedImage _entityMSAAImage;
		vktypes::AllocatedImage _viewportImage;

		vktypes::AllocatedImage lightbulb_image;
		vktypes::AllocatedImage sun_image;
		vktypes::AllocatedImage spotlight_image;

		vktypes::AllocatedBuffer _cameraDataBuffer;
		vktypes::AllocatedBuffer _lightingDataBuffer;
		vktypes::AllocatedBuffer _colorBuffer;

		MeshBuffer testmesh;
		MeshBuffer quadData;
		MeshBuffer gridmesh;
		MeshBuffer arrowmesh;
		MeshBuffer simplespheremesh;
		MeshBuffer spotmesh;

		// example pipeline
		ShaderPass _standardPass;

		// hard coded pipelines for editor only
		VkDescriptorSetLayout _flatDS0Layout = VK_NULL_HANDLE;

		VkPipelineLayout _flatcolorPipeLayout = VK_NULL_HANDLE;

		VkPipeline _flatcolorTriPipeline = VK_NULL_HANDLE;
		VkPipeline _flatcolorLinePipeline = VK_NULL_HANDLE;
		VkDescriptorSetLayout _flatcolorDS1Layout = VK_NULL_HANDLE;

		VkPipeline _flatimagePipeline = VK_NULL_HANDLE;
		VkPipelineLayout _flatimagePipeLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout _flatimageDS1Layout = VK_NULL_HANDLE;

		VkDescriptorSetLayout _globalDS0Layout = VK_NULL_HANDLE;
		VkPipelineLayout _globalPipeLayout = VK_NULL_HANDLE;

		VkDescriptorSet _viewportImageDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSet _testimage;
	};
}
