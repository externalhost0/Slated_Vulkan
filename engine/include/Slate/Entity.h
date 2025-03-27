//
// Created by Hayden Rivas on 1/21/25.
//
#pragma once
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

#include "Debug.h"
#include "Scene.h"

namespace Slate {
	class Entity {
	public:
		Entity() = delete;
		~Entity() = default;

		// proper construction involved feeding it a entt handle and assigning it a ref to scene
		Entity(const entt::entity& handle, const std::weak_ptr<Scene>& scene);

		// disallow the ability to assign entities
		Entity& operator = (const Entity&) = delete;
	public:
		bool operator == (const Entity& other) const { return this->handle == other.handle; }
		bool operator != (const Entity& other) const { return !(*this == other); }
		explicit operator bool() const { return this->handle != entt::null; }
	public:
		std::string GetName() const;
		void SetName(const std::string& new_name);
		entt::entity GetHandle() const;

		std::vector<Entity> GetChildren();
		std::weak_ptr<Entity> GetParent();
		Entity* GetRoot();

		void SetParent(std::weak_ptr<Entity> entity);

		bool HasChildren() const;
		bool HasParent() const;

		void AddChild(const Entity& entity);
		void RemoveChild(const Entity& entity);

	public:
		template<class T>
		bool HasComponent() const {
			Shared<Scene> scene = this->sceneRef.lock();
			if (!scene) return false;
			return scene->GetRegistry().all_of<T>(this->handle);
		}
		template<class T>
		T& GetMutableComponent() {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			Shared<Scene> scene = this->sceneRef.lock();
			return scene->GetRegistry().get<T>(this->handle);
		}
		template<class T>
		const T& GetImmutableComponent() const {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			Shared<Scene> scene = this->sceneRef.lock();
			return scene->GetRegistry().get<T>(this->handle);
		}
		template<class T, typename... Args>
		T& AddComponent(Args&&... args) const {
			if (HasComponent<T>()) throw std::runtime_error("Entity already has that component!");
			Shared<Scene> scene = this->sceneRef.lock();
			T& comp = scene->GetRegistry().emplace<T>(this->handle, std::forward<Args>(args)...);
			// so far we havent had any use of our own functions doing anything special when components are added
//			scene->OnComponentAdded<T>(this, comp);
			return comp;
		}
		template<class T>
		void RemoveComponent() {
			if (HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			Shared<Scene> scene = this->sceneRef.lock();
			scene->GetRegistry().remove<T>(this->handle);
		}
	private:
		std::string name;

		entt::entity handle = entt::null;
		std::weak_ptr<Scene> sceneRef;

		std::weak_ptr<Entity> parent;
		std::vector<Entity> children;

		friend class Scene;
	};
}