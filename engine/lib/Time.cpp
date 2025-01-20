//
// Created by Hayden Rivas on 1/8/25.
//
#include "Slate/Time.h"

namespace Slate {
	void Time::UpdateDeltaTime() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> delta = currentTime - lastTime;
		lastTime = currentTime;
		deltaTime = delta.count();
	}

	double Time::GetDeltaTime() { return deltaTime; }
	double Time::GetTime() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double>(currentTime - startTime).count();
	}
	void Time::Shutdown() {

	}
}