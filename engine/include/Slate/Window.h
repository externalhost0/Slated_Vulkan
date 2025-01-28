//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <utility>

#include "Expect.h"

namespace Slate {
	enum class VIDEO_MODE : unsigned char {
		FULLSCREEN,
		BORDERLESS_FULLSCREEN,
		WINDOWED,
	};

	struct WindowSpecification {
		bool IsResizeable {false};
		unsigned int WindowWidth {1280}, WindowHeight {720};
		std::string WindowTitle {"Untitled Window"};
		VIDEO_MODE VideoMode {VIDEO_MODE::WINDOWED};
	};

	class Window {
	public:
		Window() = default;
		~Window() = default;
		explicit Window(WindowSpecification spec) : m_Spec(std::move(spec)) {}

		explicit operator bool() const {
			return (m_NativeWindow != nullptr);
		}
	public:
		void Build();
		void Destroy();

		GLFWwindow* GetNativeWindow() const {
			if (m_NativeWindow)
				return m_NativeWindow;
			else
				fprintf(stderr, "Window has not been built!\n");
			return nullptr;
		}

		WindowSpecification GetSpec() {
			return m_Spec;
		}
		void SetSpecification(WindowSpecification spec) {
			m_Spec = std::move(spec);
		}
	private:
		WindowSpecification m_Spec {};
		GLFWwindow* m_NativeWindow {nullptr};
	};

//	class WindowSystem : BaseSystem {
//	public:
//		Window* GetMainWindow() {
//			if (_pWindow) return _pWindow;
//			EXPECT(false, "A main window has not been injected!")
//			return nullptr;
//		}
//	public:
//		void InjectWindow(Window* window) { _pWindow = window; };
//	private:
//		Window* _pWindow = nullptr;
//	private:
//		void Initialize() override;
//		void Shutdown() override;
//		friend class Application;
//	};
}