//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <Slate/Application.h>
#include <Slate/Scene.h>
#include <Slate/Entity.h>

#include "EditorGui.h"
#include "ViewportCamera.h"
#include "Context.h"

namespace Slate {
	struct MyGlobals {
		Window window;
		EditorGui editorGui;
		Context activeContxt;
		ViewportCamera camera;
		VulkanEngine engine;
	};

	class EditorApplication : public Application {
	private:
		void Initialize() override;
		void Loop() override;
		void Shutdown() override;
	private: // all this stuff should be better held
		bool isMinimized = false;
		MyGlobals _globals;

		vktypes::AllocatedImage lightbulb_image;
		vktypes::AllocatedImage sun_image;
		vktypes::AllocatedImage spotlight_image;
		vktypes::MeshData quadData;

		VkDescriptorSet _perSceneDescSet;  // set = 0
		VkDescriptorSet _perShaderDescSet; // set = 1
		VkDescriptorSet _perObjectDescSet; // set = 2

		VkDescriptorSetLayout _perSceneDescLayout; // binded once
		VkDescriptorSetLayout _perShaderDescLayout; // binded a couple times
		VkDescriptorSetLayout _perObjectDescLayout; // binded alot

		vktypes::MeshData gridmesh;
		vktypes::MeshData arrowmesh;
		vktypes::MeshData simplespheremesh;
		vktypes::MeshData spotmesh;


		void StandardDrawMeshes();
		void StandardaBitBiggerDrawMeshes();
	public:
		void App_OnKey(int key, int action, int mods);
		void App_OnMouseButton();
		void App_OnMouseMove();
		void App_OnWindowResize();
	};
}
