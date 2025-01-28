//
// Created by Hayden Rivas on 1/19/25.
//

#pragma once
#include <Slate/Input.h>
#include <glm/glm.hpp>
namespace Slate {
	class ViewportCamera {
	public:
		void ProcessKeys();
		void ProcessMouse(int xpos, int ypos);

		void Update();
		void OnResize(int width, int height);
		void OnResize(float width, float height);
	public:
		glm::mat4 GetProjectionMatrix() { return _projectionMatrix; };
		glm::mat4 GetViewMatrix() { return _viewMatrix; };
	private:
		glm::mat4 _projectionMatrix;
		glm::mat4 _viewMatrix;
	private:
		const float MOUSE_SENSITIVITY = 0.1f;
	public:
		float cameraSpeed = 5.f;
		bool isFirstMouse = false;
	private: // for mouse operations
		float yaw;
		float pitch;
		int lastx, lasty;
	private:
		float _zFar = 100.f;
		float _zNear = 0.1f;
		float _fov = 65.f;
		float _aspectRatio;
	private:
		glm::vec3 _position = {0.f, 0.f, 5.f}; // we like to be started back a bit so we can see things at origin easily when debugging
		glm::vec3 _front = {0, 0, -1};
		glm::vec3 _up = {0, 1, 0}; // (static) will always be this vector

		friend class EditorGui;
	};
}