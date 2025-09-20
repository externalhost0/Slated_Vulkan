//
// Created by Hayden Rivas on 1/27/25.
//

#pragma once
#include "Slate/ECS/Entity.h"
#include "Slate/ECS/Scene.h"

namespace Slate {
	template<typename... T>
	std::vector<GameEntity> Scene::GetAllEntitiesWithEXT() {
		const auto handles = this->_registry.view<T...>();
		std::vector<GameEntity> result;
		result.reserve(static_cast<size_t>(std::distance(handles.begin(), handles.end()))); // not going to work, delete
		for (entt::entity handle : handles) {
			result.emplace_back(handle, this->_registry);
		}
		return result;
	}
}