//
// Created by Hayden Rivas on 3/13/25.
//
#include "Slate/Logger.h"
#include "Slate/Entity.h"

#include <string>


namespace Slate {
	// PUBLIC METHODS //
	void Entity::SetName(const std::string& new_name) {
		this->_name = new_name;
	}
	bool Entity::HasChildren() const {
		return !this->_childrenhandles.empty();
	}
	bool Entity::HasParent() const {
		return this->_parent.has_value();
	}
	// recurse until it doesnt have a parent, this must be top level
	Entity& Entity::GetRoot() {
		if (!this->HasParent()) {
			return *this;
		}
		if (auto parent = _parent.value()) {
			return parent->GetRoot();
		}
		throw RUNTIME_ERROR("This should never happen | For entity ({})", _name);
	}
	void Entity::AddChild(const entt::entity& handle) {
		if (handle == this->_handle) {
			LOG_USER(LogLevel::Warning, "You attempted to add an Entity as a child of itself.");
			return;
		}
		if (std::find(_childrenhandles.begin(), _childrenhandles.end(), handle) != _childrenhandles.end()) {
			LOG_USER(LogLevel::Warning, "Entity is already a child of the currently parented entity.");
			return;
		}
		_childrenhandles.push_back(handle);
		if (auto scene = _scene.lock()) {
			scene->GetEntity(handle).SetParent(this);
		} else {
			LOG_USER(LogLevel::Error, "Scene expired, this shouldn't happen!");
		}
	}
	void Entity::RemoveChild(const entt::entity& handle) {
		this->_childrenhandles.erase_value(handle);
	}

	// PRIVATE METHODS //
	void Entity::SetParent(Entity* parent) {
		if (parent->_handle == _handle) {
			LOG_USER(LogLevel::Warning, "Attempted to set an Entity's parent as itself.");
			return;
		}
		_parent = parent;
	}
	Entity* Entity::GetParentPtr() {
		if (_parent.has_value()) {
			return _parent.value();
		}
		return nullptr;
	}
}