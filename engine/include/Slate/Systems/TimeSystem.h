//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "ISystem.h"
#include <chrono>
namespace Slate {
	class TimeSystem : public ISystem {
	public:
		void UpdateTime();
		double GetDeltaTime() const { return this->deltaTime; }
		double GetElapsedTime() const;
	private:
		void StartupImpl() override;
		void ShutdownImpl() override;

		std::chrono::high_resolution_clock::time_point startTime;
		double deltaTime;
		std::chrono::high_resolution_clock::time_point lastTime;
	};
}