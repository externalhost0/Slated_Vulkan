//
// Created by Hayden Rivas on 11/27/24.
//
#pragma once

#include <atomic>

#include "Slate/GX.h"
#include "Slate/Timer.h"
#include "Slate/Window.h"

namespace Slate {
	class Application {
	public:
		Application() = default;
		virtual ~Application() = default;
		// always basically called at main() as the executable
		inline virtual void run() final {
			start();
			while (_running) {
				loop();
			}
			stop();
		}
		virtual void callStop() final { _running = false; };
	public:
//		virtual void onWindowMouseButton() {};
//		virtual void onWindowMouseScroll() {};
//		virtual void onWindowKey()         {};
		virtual void onWindowClose()       {};
		virtual void onWindowFocus()       {};
		virtual void onWindowMinimize()    {};
		virtual void onWindowMaximize()    {};
		virtual void onWindowResize(uint16_t width, uint16_t height) {};
		virtual void onWindowMove(uint16_t xpos, uint16_t ypos) {};

		virtual void onSwapchainResize() {};
	protected:
		virtual void onInitialize() = 0;
		virtual void onTick()       = 0;
		virtual void onRender()     = 0;
		virtual void onShutdown()   = 0;

		// function
		inline virtual void createWindow(const WindowSpec& spec) final {
			_window.create(spec);
			installAppCallbacksToWindow(_window.getGLFWWindow());
		}

//		inline virtual ScriptEngine& getScripting() final { return _script; }
//		inline virtual GX& getGX() final { return this->_gx; }
//		inline virtual Window* getActiveWindow() final { return this->_gx._windowService.getFocusedWindow(); }
//		inline virtual InputHandler& GetInput() final { return this->_gx._windowService._input; }
//		inline virtual const Timer& getTime() const final { return this->_apptime; }
	private:
		virtual void start() final;
		virtual void loop() final;
		virtual void stop() final;

		virtual void installAppCallbacksToWindow(GLFWwindow* window) final;
	protected: // was private
		std::atomic<bool> _running = true;
		// every app has its graphics
		GX _gx;
		Window _window;
		// time
		Timer _apptime;
	};
}

