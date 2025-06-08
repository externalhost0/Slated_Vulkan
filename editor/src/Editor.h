//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <Slate/IApplication.h>

#include <Slate/ECS/Entity.h>
#include <Slate/ECS/Scene.h>
#include <Slate/SmartPointers.h>
#include <Slate/Window.h>
#include <Slate/ResourcePool.h>

#include "ViewportCamera.h"

#include "Slate/Network/Socket.h"
#include "Slate/ResourceRegistry.h"

#include <imgui.h>
#include <ImGuizmo.h>

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
		Optional<GameEntity> activeEntity = std::nullopt;
		Optional<GameEntity> hoveredEntity = std::nullopt;
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
		void _displayEntityNode(GameEntity entity, int index);
		void _onViewportPanelUpdate();
		void _onScenePanelUpdate();
		void _onPropertiesPanelUpdate();
		void _onAssetPanelUpdate();

		void _settingsUpdate();

		void _createVisualizerMeshes();

		bool IsMouseInViewportBounds();
		void InitImGui(ImGuiRequiredData req, GLFWwindow* glfwWindow, VkFormat format);
	public:
		InternalTextureHandle colorResolveImage;
		InternalTextureHandle colorMSAAImage;
		InternalTextureHandle depthStencilMSAAImage;
		InternalTextureHandle entityResolveImage;
		InternalTextureHandle entityMSAAImage;
		InternalTextureHandle viewportImage;
		InternalTextureHandle outlineImage;

		VkDescriptorSet _fileImageDS;
		VkDescriptorSet _folderImageDS;

		InternalBufferHandle _stagbuf;

		MeshData arrowmesh;
		MeshData simplespheremesh;
		MeshData spotmesh;


		Optional<std::filesystem::path> _selectedEntry;
		std::filesystem::path _assetsDirectory = "../../editor";
		std::filesystem::path _currentDirectory = _assetsDirectory;


		ResourceRegistry registry;
		ResourcePool<MeshResource> _meshPool;
		ResourcePool<ScriptResource> _scriptPool;
		ResourcePool<ShaderResource> _shaderPool;
		ResourcePool<TextureResource> _texturePool;



		glm::vec2 _viewportSize{ 2280, 1720 };
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


		VkDescriptorSet _viewportImageDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorPool _imguiDescriptorPool = VK_NULL_HANDLE;
	};
}
