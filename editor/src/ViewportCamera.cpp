//
// Created by Hayden Rivas on 1/19/25.
//

#include "ViewportCamera.h"
#include "Slate/Input.h"
#include <Slate/Time.h>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Slate {
	void ViewportCamera::OnResize(int width, int height) {
		_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	}
	void ViewportCamera::ProcessKeys(InputSystem& system) {

		float realSpeed;
		if (system.IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) realSpeed = CAMERA_BASE_SPEED;
		else realSpeed = CAMERA_SPRINT_SPEED;

		float adjustedSpeed = realSpeed * static_cast<float>(Time::GetDeltaTime());

		if (system.IsKeyPressed(GLFW_KEY_W))
			_position += adjustedSpeed * _front;
		if (system.IsKeyPressed(GLFW_KEY_S))
			_position -= adjustedSpeed * _front;
		if (system.IsKeyPressed(GLFW_KEY_A))
			_position -= glm::normalize(glm::cross(_front, _up)) * adjustedSpeed;
		if (system.IsKeyPressed(GLFW_KEY_D))
			_position += glm::normalize(glm::cross(_front, _up)) * adjustedSpeed;
		if (system.IsKeyPressed(GLFW_KEY_SPACE))
			_position += adjustedSpeed * _up;
		if (system.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
			_position -= adjustedSpeed * _up;
	}


	void ViewportCamera::ProcessMouse(int xpos, int ypos) {
		// first part: resolve mouse movement
		if (isFirstMouse) {
			lastx = xpos;
			lasty = ypos;
			isFirstMouse = false;
		}

		int xoffset = xpos - lastx;
		int yoffset = lasty - ypos; // reversed since y-coordinates go from bottom to top

		lastx = xpos;
		lasty = ypos;
		// second part: calculate new front facing vector
		yaw += static_cast<float>(xoffset) * MOUSE_SENSITIVITY;
		pitch += static_cast<float>(yoffset) * MOUSE_SENSITIVITY;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.z = -cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
		direction.y = sinf(glm::radians(pitch));
		direction.x = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
		_front = glm::normalize(direction);
	}
	void ViewportCamera::Update() {
		auto proj = glm::perspective(glm::radians(_fov), _aspectRatio, _zNear, _zFar);
		proj[1][1] *= -1; // inverts the y axis
		_projectionMatrix = proj;

		_viewMatrix = glm::lookAt(_position, _position + _front, _up);
	}
}