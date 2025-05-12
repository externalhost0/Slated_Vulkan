//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include <unordered_map>
#include <memory>
#include <typeindex>

#include "Slate/Common/Logger.h"
#include "Slate/SmartPointers.h"
#include "Slate/Systems/ISystem.h"

namespace Slate {
	class SystemManager {
	public:
		template<class T>
		void registerSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem");
			UniquePtr<T> system = CreateUniquePtr<T>();
			if (std::find(ordered_systems.begin(), ordered_systems.end(), system) == ordered_systems.end()) {
				ordered_systems.push_back(std::move(system));
			}
		}
		template<class T>
		void deregisterSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem");
			ordered_systems.erase(
					std::remove_if(
							ordered_systems.begin(),
							ordered_systems.end(),
							[](const UniquePtr<ISystem>& system) {
								return dynamic_cast<T*>(system.get()) != nullptr;
							}),
					ordered_systems.end()
			);
		}
		void startAll(Scene& scene) {
			for (auto& system : ordered_systems) {
				system->start(scene);
			}
		}
		void updateAll(Scene& scene) {
			for (auto& system : ordered_systems) {
				system->update(scene);
			}
		}
		void stopAll(Scene& scene) {
			for (auto& system : ordered_systems) {
				system->stop(scene);
			}
		}
	private:
		std::vector<UniquePtr<ISystem>> ordered_systems;
	};

}