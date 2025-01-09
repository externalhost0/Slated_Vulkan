//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <string>
#include <GLFW/glfw3.h>

#include "Ref.h"

#include "IManager.h"
#include "Expect.h"

namespace Slate {
	enum class VIDEO_MODE : unsigned char {
		FULLSCREEN,
		BORDERLESS_FULLSCREEN,
		WINDOWED,
	};

	struct WindowSpecification {
		bool IsResizeable{false};
		bool VSyncEnabled{false};

		unsigned int RefreshRate{}; // leave empty for the mode to figure out
		unsigned int WindowWidth{1720}, WindowHeight{1280};

		std::string WindowTitle{"Untitled Window"};
		VIDEO_MODE VideoMode{VIDEO_MODE::WINDOWED};

	};
	class Window {
	public:
		Window() = default;
		~Window() = default;

		explicit operator bool() const {
			return (m_NativeWindow != nullptr);
		}

		explicit Window(WindowSpecification spec)
		: m_Spec(std::move(spec)) {}
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

	private:
		WindowSpecification m_Spec;
		GLFWwindow* m_NativeWindow{nullptr};

	};

	class WindowManager : public IManager {
	public:
		Ref<Window> GetWindow() {
			if (m_Window)
				return m_Window;
			else
				EXPECT(false, "A main window has not been set!")
		}
		void SetMainWindow(const Window& window) { m_Window = CreateRef<Window>(window); }

	private:
		void Initialize() override {};
		void Shutdown() override {};
		friend class Application;
	private:
		Ref<Window> m_Window;
	};
}