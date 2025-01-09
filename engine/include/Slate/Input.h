//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

#include <glm/glm.hpp>
#include "IManager.h"

namespace Slate {
	class InputManager : public IManager {
	public:

		bool IsKeyPressed(int key);
		bool IsMouseButtonPressed(int button);
		glm::vec2 GetMousePosition();

		void SetInputMode(int mode);
		int GetInputMode();
	private:
		void Initialize() override {};
		void Shutdown() override {};
		friend class Application;
	private:
	};

}