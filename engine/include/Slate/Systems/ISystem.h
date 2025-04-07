//
// Created by Hayden Rivas on 1/14/25.
//

#pragma once
#include "Slate/Logger.h"

namespace Slate {
	class SystemManager;

	class ISystem {
	public:
		virtual inline void Startup() final {
			if (this->owner == nullptr) {
				LOG_USER(LogLevel::Error, "A systemManager was not attached to the system.");
				return;
			}
			if (this->isActive) {
				LOG_USER(LogLevel::Warning, "Startup can only be called if it hasn't ran before!");
				return;
			}
			this->StartupImpl();
			this->isActive = true;
		}
		virtual inline void Shutdown() final {
			if (!this->isActive) {
				LOG_USER(LogLevel::Warning, "Shutdown can only be called if system has Started!");
				return;
			}
			this->ShutdownImpl();
			this->isActive = false;
		}
		virtual ~ISystem() = default;

//		virtual void Update(entt::registry registry) = 0;
	protected:
		ISystem() = default;

		virtual void StartupImpl() = 0;
		virtual void ShutdownImpl() = 0;

		SystemManager* owner = nullptr;
	private:
		bool isActive = false;
		friend class SystemManager;
	};
}