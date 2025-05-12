//
// Created by Hayden Rivas on 3/13/25.
//
#include "Slate/ECS/Entity.h"
#include "Slate/Common/Logger.h"

#include <vector>
#include <string>


namespace Slate {
	// PUBLIC METHODS //
	void GameEntity::SetName(const std::string& new_name) const {
		Core().name = new_name;
	}
	const std::string& GameEntity::GetName() const {
		return Core().name;
	}
	bool GameEntity::HasChildren() const {
		return !Core().children.empty();
	}
	bool GameEntity::HasParent() const {
		return !(Core().parent == entt::null);
	}
	void GameEntity::AddChild(GameEntity entity) {
		if (entity.GetHandle() == this->_handle) {
			LOG_USER(LogType::Warning, "You attempted to add an Entity as a child of itself.");
			return;
		}
		if (std::find(Core().children.begin(), Core().children.end(), entity.GetHandle()) != Core().children.end()) {
			LOG_USER(LogType::Warning, "Entity is already a child of the currently parented entity.");
			return;
		}
		Core().children.push_back(entity.GetHandle());
		_registry.get<CoreComponent>(entity.GetHandle()).parent = this->_handle;
	}
	void GameEntity::RemoveChild(GameEntity entity) {
		Core().children.erase_value(entity.GetHandle());
		_registry.get<CoreComponent>(entity.GetHandle()).parent = entt::null;
	}
	GameEntity GameEntity::GetParent() {
		return {Core().parent, this->_registry};
	}
	std::vector<GameEntity> GameEntity::GetChildren() {
		std::vector<GameEntity> result;
		result.reserve(Core().children.size());
		for (entt::entity child : Core().children) {
			result.emplace_back(child, this->_registry);
		}
		return result;
	}
	GameEntity GameEntity::GetRoot() {
		if (!this->HasParent()) {
			return *this;
		}
		return this->GetParent().GetRoot();
	}
}