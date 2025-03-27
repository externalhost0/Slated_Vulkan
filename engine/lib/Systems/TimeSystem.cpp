//
// Created by Hayden Rivas on 3/19/25.
//
#include "Slate/Systems/TimeSystem.h"
#include <chrono>
namespace Slate {
	void TimeSystem::StartupImpl() {
		this->startTime = std::chrono::high_resolution_clock::now();
		this->lastTime = this->startTime;
	}
	void TimeSystem::ShutdownImpl() {

	}
	void TimeSystem::UpdateTime() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> delta = currentTime - this->lastTime;
		this->lastTime = currentTime;
		this->deltaTime = delta.count();
	}

	double TimeSystem::GetElapsedTime() const {
		auto currentTime = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(currentTime - this->startTime).count();
	}
}