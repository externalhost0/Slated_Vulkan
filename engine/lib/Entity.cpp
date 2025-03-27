//
// Created by Hayden Rivas on 3/13/25.
//
#include <string>
#include <utility>

#include "Slate/Entity.h"
namespace Slate {
	Entity::Entity(const entt::entity& handle, const std::weak_ptr<Scene>& scene) {
		this->handle = handle;
		this->sceneRef = scene;
	}
	std::string Entity::GetName() const {
		return this->name;
	}
	void Entity::SetName(const std::string& new_name) {
		this->name = new_name;
	}
	entt::entity Entity::GetHandle() const {
		return this->handle;
	}
	std::vector<Entity> Entity::GetChildren() {
		return this->children;
	}
	std::weak_ptr<Entity> Entity::GetParent() {
		return this->parent;
	}
	Entity* Entity::GetRoot() {
		return nullptr;
	}
	void Entity::SetParent(std::weak_ptr<Entity> entity) {
		if (entity.lock()->handle == this->handle) {
			return;
		}
		this->parent = std::move(entity);
	}
	bool Entity::HasChildren() const {
		return (!this->children.empty());
	}
	bool Entity::HasParent() const {
		return false;
	}
	void Entity::AddChild(const Entity& entity) {
		if (entity.handle == this->handle) {
			return;
		}
		if (std::find(this->children.begin(), this->children.end(), entity) != this->children.end()) {
			this->children.push_back(entity);
		}
	}
	void Entity::RemoveChild(const Entity& entity) {
//		this->children.erase(entity.get()->GetHandle());
	}

}