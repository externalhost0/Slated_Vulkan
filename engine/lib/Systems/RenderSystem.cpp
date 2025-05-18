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
		GeometryPrimitiveComponent& geometry_c = entity.getComponent<GeometryPrimitiveComponent>();
		DrawPrimitive(geometry_c);
		if (entity.hasChildren()) {
			for (GameEntity child : entity.getChildren()) {
				RenderPrimitive(child);
			}
		}
	}

	void RenderSystem::onStart(Scene& scene) {

	}
	void RenderSystem::onUpdate(Scene& scene) {
		for (GameEntity entity : scene.GetRootEntities()) {
			if (entity.hasComponent<GeometryPrimitiveComponent>()) {
//				RenderPrimitive(entity);
			} else if (entity.hasComponent<GeometryGLTFComponent>()) {

			}
		}
	}
	void RenderSystem::onStop(Scene& scene) {

	}

}