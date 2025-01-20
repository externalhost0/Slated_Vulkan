//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <chrono>

namespace Slate {
	class Time {
	public:
		// used anywhere
		static double GetDeltaTime();
		static double GetTime();
	public: // just to say that it cant be inistaited in any way
		Time() = delete;
		Time(const Time&) = delete;
		Time(Time&&) = delete;
		Time& operator=(const Time&) = delete;
		Time& operator=(Time&&) = delete;
	private:
		inline static double deltaTime;
		inline static std::chrono::high_resolution_clock::time_point startTime;
		inline static std::chrono::high_resolution_clock::time_point lastTime;
	private:
		// we ONLY want this to be calculated automatically inside the application class and not touched anywhere else
		static void UpdateDeltaTime();
		// same goes for its setting of times
		static void Initialize() {
			startTime = std::chrono::high_resolution_clock::now();
			lastTime = startTime;
		}
		friend class Application;
		static void Shutdown();
	};
}