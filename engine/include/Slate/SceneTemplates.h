//
// Created by Hayden Rivas on 1/27/25.
//

#pragma once
#include "Scene.h"
#include "Entity.h"

namespace Slate {
	template<typename... Components>
	auto Scene::GetAllIDsWith() {
		return _entityRegistry.view<Components...>();
	}

	template<typename... Components>
	std::vector<Entity> Scene::GetAllEntitiesWith() {
		auto ids = this->GetAllIDsWith<Components...>();
		std::vector<Entity> result;
		result.reserve(static_cast<size_t>(std::distance(ids.begin(), ids.end()))); // not going to work, delete
		for (const entt::entity& id : ids) {
			result.emplace_back(id, this->_entityRegistry);
		}
		return result;
	}


}