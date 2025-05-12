//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "Slate/Common/FastSTD.h"
#include "Slate/InputHandler.h"
#include "Slate/SmartPointers.h"
#include "Slate/Window.h"

#include <optional>
#include <unordered_map>

namespace Slate {
	class WindowService final {
	public:
		~WindowService();

		WindowHandle createWindow(const WindowSpec& spec = {});
		void destroyWindow(WindowHandle window);
		void focusWindow(WindowHandle handle);

		Window* getFocusedWindow();
		Window* getWindowFromHandle(WindowHandle handle);
	private:
		WindowHandle _nextHandle;
		Optional<WindowHandle> _focusedHandle;
		std::unordered_map<WindowHandle, UniquePtr<Window>> _registeredWindows;

		InputHandler input{*this};
		friend class IApplication;
	};
}