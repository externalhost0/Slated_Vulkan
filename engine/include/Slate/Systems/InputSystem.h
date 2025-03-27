//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "ISystem.h"

#include <tuple>
#include <GLFW/glfw3.h>

namespace Slate {
	enum class MouseShape {
		CROSSHAIR,
		CENTER
	};
	enum class InputMode {
		CURSOR_DISABLED,
		CURSOR_NORMAL
	};

	class InputSystem : public ISystem {
	public:

		bool IsKeyPressed(int key, int keystate = GLFW_PRESS) const;
		bool IsMouseButtonPressed(int button) const;

		std::pair<float, float> GetMousePosition() const;

		InputMode GetInputMode() const;
		void SetInputMode(InputMode new_mode);
	private:
		void StartupImpl() override;
		void ShutdownImpl() override;

	};
}




