//
// Created by Hayden Rivas on 1/27/25.
//

#pragma once
#include "Scene.h"
#include "Entity.h"

namespace Slate {
	template<typename... T>
	std::vector<Shared<Entity>> Scene::GetAllEntitiesWith() {
		const auto& ids = this->registry.view<T...>();
		std::vector<Shared<Entity>> entites_to_grab;
		entites_to_grab.reserve(static_cast<size_t>(std::distance(ids.begin(), ids.end()))); // not going to work, delete
		for (const entt::entity& handle : ids) {
			entites_to_grab.emplace_back(this->entityMap[handle]);
		}
		return entites_to_grab;
	}
}