//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "ISystem.h"
#include "Slate/Window.h"
#include "Slate/SmartPointers.h"

#include <vector>

namespace Slate {
	class WindowSystem : public ISystem {
	public:
		StrongPtr<Window> GetCurrentWindow() {
			if (this->activeWindow.has_value()) {
				return this->activeWindow.value();
			} else {
				return nullptr;
			}
		}

	private:
		void StartupImpl() override;
		void ShutdownImpl() override;

		Optional<StrongPtr<Window>> activeWindow;

		std::vector<StrongPtr<Window>> registeredWindows;
	};
}