//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <Slate/Application.h>
#include "EditorGui.h"
#include "ViewportCamera.h"

namespace Slate {
	class EditorApplication : public Application {
	private:
		void Initialize() override;
		void Loop() override;
		void Shutdown() override;
	private:
		bool isMinimized = false;
		EditorGui _editorGui;
		Ref<ViewportCamera> _camera;
	private:

	};
}
