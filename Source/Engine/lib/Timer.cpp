//
// Created by Hayden Rivas on 3/19/25.
//
#include "Slate/Timer.h"

#include <chrono>
namespace Slate {
	Timer::Timer() {
		this->_startTime = Clock::now();
		this->_lastTime = this->_startTime;
	}

	void Timer::update() {
		if (!_isPaused) {
			auto currentTime = Clock::now();
			_deltaTime = std::chrono::duration_cast<Duration>(currentTime - _lastTime).count();
			_lastTime = currentTime;
		} else {
			_deltaTime = 0.f;
		}
	}
	void Timer::pause() {
		if (!_isPaused) {
			_isPaused = true;
		}
	}
	void Timer::resume() {
		if (_isPaused) {
			_isPaused = false;
			_startTime += Clock::now() - _pauseTime;
			_lastTime = Clock::now();
		}
	}
	void Timer::reset() {
		_startTime = Clock::now();
		_lastTime = _startTime;
		_deltaTime = 0.f;
		_isPaused = false;
	}

	double Timer::getDeltaTime() const {
		return _deltaTime;
	}
	double Timer::getElapsedTime() const {
		if (_isPaused) {
			return std::chrono::duration_cast<Duration>(_pauseTime - _startTime).count();
		}
		return std::chrono::duration_cast<Duration>(Clock::now() - _startTime).count();
	}
}