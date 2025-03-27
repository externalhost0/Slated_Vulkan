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
			if (this->isActive) {
				LOG_USER(LogLevel::Warning, "Startup can only be called if it hasn't ran before!");
			}
			this->StartupImpl();
			this->isActive = true;
		}
		virtual inline void Shutdown() final {
			if (!this->isActive) {
				LOG_USER(LogLevel::Warning, "Shutdown can only be called if system has Started!");
			}
			this->ShutdownImpl();
			this->isActive = false;
		}
		virtual ~ISystem() = default;

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