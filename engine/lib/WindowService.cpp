//
// Created by Hayden Rivas on 3/19/25.
//
#include "Slate/WindowService.h"
#include "Slate/Window.h"

namespace Slate {
	WindowService::~WindowService() {
		_registeredWindows.clear();
	}
	WindowHandle WindowService::createWindow(const WindowSpec& spec) {
		UniquePtr<Window> window = CreateUniquePtr<Window>();
		window->create(spec);
		WindowHandle handle = _nextHandle++;
		_registeredWindows.emplace(handle, std::move(window));
		if (!_focusedHandle.has_value()) {
			_focusedHandle = handle;
		}
		return handle;
	}
	void WindowService::destroyWindow(WindowHandle handle) {
		if (_registeredWindows.erase(handle)) {
			if (_focusedHandle == handle) {
				_focusedHandle = std::nullopt;
			}
		}
	}
	void WindowService::focusWindow(WindowHandle handle) {
		if (_registeredWindows.find(handle) != _registeredWindows.end()) {
			_focusedHandle.emplace(handle);
		}
	}
	Window* WindowService::getWindowFromHandle(WindowHandle handle) {
		auto it = _registeredWindows.find(handle);
		return (it != _registeredWindows.end()) ? it->second.get() : nullptr;
	}
	Window* WindowService::getFocusedWindow() {
		return _registeredWindows[_focusedHandle.value()].get();
	}
}