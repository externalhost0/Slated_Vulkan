//
// Created by Hayden Rivas on 3/16/25.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Slate {
	class Camera {
	public:
		enum class ProjectionType {
			Perspective,
			Orthographic
		} projectionType = ProjectionType::Perspective;

		void UpdateMatrices() {
			this->viewMatrix = glm::lookAt(this->position, this->position + this->frontVector, this->upVector);
			switch (this->projectionType) {
				case ProjectionType::Perspective:
					this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspectRatio, this->near, this->far);
					break;
				case ProjectionType::Orthographic:
					this->projectionMatrix = glm::ortho(this->left, this->right, this->bottom, this->top, this->near, this->far);
					break;
			}
		};
		void UpdateAspect(int w, int h) {
			if (w/h <= 0) return;
			this->aspectRatio = (float)w / (float)h;
		}
		void UpdateAspect(float w, float h) {
			if (w/h <= 0) return;
			this->aspectRatio = w / h;
		}
	public:
		glm::mat4 GetProjectionMatrix() const { return this->projectionMatrix; }
		glm::mat4 GetViewMatrix() const { return this->viewMatrix; }

		glm::vec3 GetUpVector() const { return this->upVector; }
		glm::vec3 GetFrontVector() const { return this->frontVector; }
		glm::vec3 GetPosition() const { return this->position; }
	public:
		float GetNearPlaneDistance() const { return this->near; }
		float GetFarPlaneDistance() const { return this->far; }
		float GetFOV() const { return this->fov; }

		void SetNearPlaneDistance(float new_near) { this->near = new_near; }
		void SetFarPlaneDistance(float new_far) { this->far = new_far; }
		void SetFOV(float new_fov) { this->fov = new_fov; }
	protected:
		glm::vec3 frontVector = {0, 0, -1};
		glm::vec3 upVector = {0, 1, 0};

		glm::vec3 position = {0.f, 0.f, 5.f};

		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;

		float near = 0.1f;
		float far = 100.f;
		float fov = 65.f;
		float aspectRatio = 1.33f; // TODO: find better way to default this
		float left, right, bottom, top;
	};
}