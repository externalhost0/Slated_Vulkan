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
	};
}
