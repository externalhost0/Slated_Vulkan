//
// Created by Hayden Rivas on 4/8/25.
//
#include "Slate/Systems/TransformSystem.h"
#include "Slate/ECS/Scene.h"

#include "Slate/ECS/Components.h"
#include "Slate/ECS/Entity.h"
namespace Slate {
	glm::mat4 TransformToMatrix(const Transform& transform) {
		return glm::translate(glm::mat4(1.f), transform.position) *
			   glm::mat4_cast(transform.rotation) *
			   glm::scale(glm::mat4(1.f), transform.scale);
	}

	void UpdateGlobalTransforms(GameEntity entity, const Transform& topTransform = {}) {
		TransformComponent& transform_c = entity.GetComponent<TransformComponent>();
		transform_c.global.position += topTransform.position;
		transform_c.global.rotation += topTransform.rotation;
		if (entity.HasChildren()) {
			for (GameEntity child : entity.GetChildren()) {
				UpdateGlobalTransforms(child, transform_c.global);
			}
		}
	}

	void TransformSystem::onStart(Scene& scene) {

	}
	void TransformSystem::onUpdate(Scene& scene) {
//		for (GameEntity entity : scene.GetRootEntities()) {
//			TransformComponent& transform_c = entity.GetComponent<TransformComponent>();
//			transform_c.local = transform_c.global;
//			UpdateGlobalTransforms(entity);
//		}
	}
	void TransformSystem::onStop(Scene& scene) {

	}
}