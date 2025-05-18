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
		};

		void updateMatrices() {
			this->_viewMatrix = glm::lookAt(_position, _position + _frontVector, _upVector);
			switch (this->projectionType) {
				case ProjectionType::Perspective:
					this->_projectionMatrix = glm::perspective(glm::radians(_fov), _aspectRatio, _near, _far);
					break;
				case ProjectionType::Orthographic:
					this->_projectionMatrix = glm::ortho(_left, _right, _bottom, _top, _near, _far);
					break;
			}
		};

		void setOrthoHeight(float h) {
			_unitSize = h;

			float halfHeight = h / 2.0f;
			float halfWidth = halfHeight * _aspectRatio;
			_left = -halfWidth;
			_right = halfWidth;
			_bottom = -halfHeight;
			_top = halfHeight;
		}
		void updateAspect(float w, float h) {
			if (w/h <= 0) return;
			this->_aspectRatio = w / h;
			if (projectionType == ProjectionType::Orthographic) {
				setOrthoHeight(_unitSize);
			}
		}
	public:
		glm::mat4 getProjectionMatrix() const { return this->_projectionMatrix; }
		glm::mat4 getViewMatrix() const { return this->_viewMatrix; }

		glm::vec3 getUpVector() const { return this->_upVector; }
		glm::vec3 getFrontVector() const { return this->_frontVector; }
		glm::vec3 getPosition() const { return this->_position; }

		float getNearPlaneDistance() const { return this->_near; }
		float getFarPlaneDistance() const { return this->_far; }
		float getFOV() const { return this->_fov; }
	public:
		void setProjectionMode(ProjectionType type) {
			this->projectionType = type;
			setOrthoHeight(_top - _bottom);
		}
		void setNearPlaneDistance(float new_near) { this->_near = new_near; }
		void setFarPlaneDistance(float new_far) { this->_far = new_far; }
		void setFOV(float new_fov) { this->_fov = new_fov; }
	protected:
		glm::vec3 _frontVector = {0, 0, -1};
		glm::vec3 _upVector = {0, 1, 0};

		glm::vec3 _position = {0.f, 0.f, 5.f};

		glm::mat4 _projectionMatrix;
		glm::mat4 _viewMatrix;

		float _near = 0.1f;
		float _far = 100.f;
		float _fov = 65.f;
		float _aspectRatio = 1.33f; // TODO: find better way to default this

		float _unitSize = 10.0f;
		float _left = -_unitSize, _right = _unitSize, _bottom = -_unitSize, _top = _unitSize;

		ProjectionType projectionType = ProjectionType::Perspective;
	};
}