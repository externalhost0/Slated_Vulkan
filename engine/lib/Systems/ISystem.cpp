//
// Created by Hayden Rivas on 4/10/25.
//
#include "Slate/Systems/ISystem.h"
#include "Slate/Common/Logger.h"

namespace Slate {
	namespace {
		constexpr const char* state_strings[] = {
				"Inactive",
				"Starting",
				"Updateing",
				"Stopping"
		};
	}
	SystemState ISystem::getState() {
		return currentState;
	}
	const char* ISystem::getStateAsString() {
		return state_strings[(int) currentState];
	}

	void ISystem::start(Slate::Scene& scene) {
		if (currentState != SystemState::Inactive) {
			LOG_USER(LogType::Warning, "Startup can only be called if is inactive!");
			return;
		}
		currentState = SystemState::Starting;
		onStart(scene);
	}
	void ISystem::update(Slate::Scene& scene) {
		currentState = SystemState::Updating;
		onUpdate(scene);
	}
	void ISystem::stop(Slate::Scene& scene) {
		if (currentState != SystemState::Updating) {
			LOG_USER(LogType::Warning, "Shutdown can only be called if system is active!");
			return;
		}
		currentState = SystemState::Stopping;
		onStop(scene);
		currentState = SystemState::Inactive;
	}
}