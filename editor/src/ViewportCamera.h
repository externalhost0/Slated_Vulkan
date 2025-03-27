//
// Created by Hayden Rivas on 1/19/25.
//

#pragma once
#include <Slate/Camera.h>
#include <glm/glm.hpp>
namespace Slate {
	class ViewportCamera : public Camera {
	public:
		void ProcessKeys(GLFWwindow* window, double deltaTime);
		void ProcessMouse(int xpos, int ypos);
	private:
		const float MOUSE_SENSITIVITY = 0.1f;
	public:
		float cameraSpeed = 6.f;
		bool isFirstMouse = false;
	private: // for mouse operations
		float yaw{};
		float pitch{};
		int lastx{}, lasty{};
		friend class Editor;
	};
}