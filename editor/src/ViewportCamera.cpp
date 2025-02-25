//
// Created by Hayden Rivas on 1/19/25.
//

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <Slate/Input.h>
#include <Slate/Time.h>
#include "ViewportCamera.h"


namespace Slate {

	void ViewportCamera::OnResize(float width, float height) {
		if (width/height <= 0) return;
		_aspectRatio = width/height;
	}
	void ViewportCamera::ProcessKeys() {

		float realSpeed;
		if (InputSystem::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) realSpeed = cameraSpeed * 1.75f;
		else realSpeed = cameraSpeed;

		float adjustedSpeed = realSpeed * static_cast<float>(Time::GetDeltaTime());

		if (InputSystem::IsKeyPressed(GLFW_KEY_W))
			_position += adjustedSpeed * _front;
		if (InputSystem::IsKeyPressed(GLFW_KEY_S))
			_position -= adjustedSpeed * _front;
		if (InputSystem::IsKeyPressed(GLFW_KEY_A))
			_position -= glm::normalize(glm::cross(_front, _up)) * adjustedSpeed;
		if (InputSystem::IsKeyPressed(GLFW_KEY_D))
			_position += glm::normalize(glm::cross(_front, _up)) * adjustedSpeed;
		if (InputSystem::IsKeyPressed(GLFW_KEY_SPACE))
			_position += adjustedSpeed * _up;
		if (InputSystem::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
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
		_projectionMatrix = glm::perspective(glm::radians(_fov), _aspectRatio, _zNear, _zFar);
//		proj[1][1] *= -1; // inverts the y axis

		_viewMatrix = glm::lookAt(_position, _position + _front, _up);
	}
}