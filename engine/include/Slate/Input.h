//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

#include <glm/glm.hpp>

#include "Ref.h"
#include "Window.h"

namespace Slate {
	class InputSystem : BaseSystem {
	public:
		bool IsKeyPressed(int key, int keystate = GLFW_PRESS);
		bool IsMouseButtonPressed(int button);
		glm::ivec2 GetMousePosition();

		void SetInputMode(int mode);
		int GetInputMode();

	public:
		void InjectWindow(const Ref<Window>& windowRef) { _windowRef = windowRef; }
	private:
		Ref<Window> _windowRef = nullptr;
	private:
		void Initialize() override;
		void Shutdown() override;
		friend class Application;
	};

}