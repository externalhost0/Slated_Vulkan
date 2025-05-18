//
// Created by Hayden Rivas on 3/13/25.
//
#include "Slate/ECS/Entity.h"
#include "Slate/Common/Logger.h"

#include <vector>
#include <string>


namespace Slate {
	// PUBLIC METHODS //
	void GameEntity::setName(const std::string& new_name) {
		_registry.get<GameEntity::Name>(_handle).name = new_name;
	}
	const std::string& GameEntity::getName() const {
		return _registry.get<GameEntity::Name>(_handle).name;
	}
	bool GameEntity::hasChildren() const {
		return !_registry.get<GameEntity::Hierarchy>(_handle).children.empty();
	}
	bool GameEntity::hasParent() const {
		return !(_registry.get<GameEntity::Hierarchy>(_handle).parent == entt::null);
	}
	void GameEntity::addChild(GameEntity entity) {
		if (entity.getHandle() == _handle) {
			LOG_USER(LogType::Warning, "You attempted to add an Entity as a child of itself.");
			return;
		}
		GameEntity::Hierarchy& hierarchy = _registry.get<GameEntity::Hierarchy>(_handle);
		if (std::find(hierarchy.children.begin(), hierarchy.children.end(), entity.getHandle()) != hierarchy.children.end()) {
			LOG_USER(LogType::Warning, "Entity is already a child of the currently parented entity.");
			return;
		}
		hierarchy.children.push_back(entity.getHandle());
		hierarchy.parent = this->_handle;
	}
	void GameEntity::removeChild(GameEntity entity) {
		GameEntity::Hierarchy& hierarchy = _registry.get<GameEntity::Hierarchy>(_handle);
		hierarchy.children.erase_value(entity.getHandle());
		hierarchy.parent = entt::null;
	}

	GameEntity GameEntity::getParent() {
		return {_registry.get<Hierarchy>(_handle).parent, _registry};
	}
	std::vector<GameEntity> GameEntity::getChildren() {
		const GameEntity::Hierarchy& hierarchy = _registry.get<GameEntity::Hierarchy>(_handle);
		std::vector<GameEntity> result;
		result.reserve(hierarchy.children.size());
		for (const entt::entity& child : hierarchy.children) {
			result.emplace_back(child, _registry);
		}
		return result;
	}
	GameEntity GameEntity::getRoot() {
		if (!this->hasParent()) {
			return *this;
		}
		return getParent().getRoot();
	}
}