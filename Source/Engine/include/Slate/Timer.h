//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include <chrono>

namespace Slate {
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	using Duration = std::chrono::duration<double>;

	class Timer final {
	public:
		Timer();

		void update();
		void resume();
		void pause();
		void reset();

		inline bool isPaused() const { return _isPaused; }

		double getDeltaTime() const;
		double getElapsedTime() const;
	private:
		TimePoint _startTime;
		TimePoint _lastTime;
		TimePoint _pauseTime;

		double _deltaTime = 0.f;
		bool _isPaused = false;
	};
}