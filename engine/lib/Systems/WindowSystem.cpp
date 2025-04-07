//
// Created by Hayden Rivas on 3/19/25.
//
#include "Slate/Systems/WindowSystem.h"
namespace Slate {
	void WindowSystem::StartupImpl() {
		registeredWindows.push_back(std::make_unique<Window>());
		activeWindow = Optional<UniquePtr<Window>>(registeredWindows[0].get());
	}
	void WindowSystem::ShutdownImpl() {
		activeWindow.reset();
		registeredWindows.clear();
	}
}