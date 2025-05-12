//
// Created by Hayden Rivas on 3/19/25.
//

#include "Slate/Systems/RenderSystem.h"
#include "Slate/ECS/Entity.h"
#include "Slate/ECS/Scene.h"

namespace Slate {

	void DrawPrimitive(GeometryPrimitiveComponent geometry_c) {

	}

	void RenderPrimitive(GameEntity entity) {
		GeometryPrimitiveComponent& geometry_c = entity.GetComponent<GeometryPrimitiveComponent>();
		DrawPrimitive(geometry_c);
		if (entity.HasChildren()) {
			for (GameEntity child : entity.GetChildren()) {
				RenderPrimitive(child);
			}
		}
	}

	void RenderSystem::onStart(Scene& scene) {

	}
	void RenderSystem::onUpdate(Scene& scene) {
		for (GameEntity entity : scene.GetRootEntities()) {
			if (entity.HasComponent<GeometryPrimitiveComponent>()) {
				RenderPrimitive(entity);
			} else if (entity.HasComponent<GeometryGLTFComponent>()) {

			}
		}
	}
	void RenderSystem::onStop(Scene& scene) {

	}

}