//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include <unordered_map>
#include <memory>
#include <typeindex>

#include "Slate/Systems/ISystem.h"
#include "SmartPointers.h"

namespace Slate {
	class SystemManager {
	public:
		template <typename T>
		void RegisterSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem");
			auto system = CreateUniquePtr<T>();
			system->owner = this;
			system->Startup();
			systems[std::type_index(typeid(T))] = std::move(system);
		}
		template <typename T>
		T* GetSystem() {
			auto it = systems.find(std::type_index(typeid(T)));
			if (it != this->systems.end()) {
				return dynamic_cast<T*>(it->second.get());
			}
			return nullptr;
		}
	private:
		std::unordered_map<std::type_index, UniquePtr<ISystem>> systems;
	};

}