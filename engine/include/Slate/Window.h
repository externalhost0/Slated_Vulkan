//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <utility>


namespace Slate {
	enum class VideoMode : char {
		FULLSCREEN,
		BORDERLESS_FULLSCREEN,
		WINDOWED,
	};

	// lets design important classes with [type]specification
	struct WindowSpecification {
		VideoMode videomode = VideoMode::WINDOWED;
		std::string title = "Untitled Window";
		bool resizeable = false;
		unsigned int width = 1280, height = 720;
	};

	class Window {
	public:
		Window() = default;
		explicit Window(WindowSpecification init_specification) : specification(std::move(init_specification)) {};
		~Window() { glfwDestroyWindow(this->glfwWindow); }

		void Initialize();
		void Shutdown();
	public:
		void PollWindow() const { glfwPollEvents(); };
		void FocusWindow() const { glfwFocusWindow(this->glfwWindow); }


		inline GLFWwindow* GetGlfwWindow()  { return this->glfwWindow; }
		inline std::string GetTitle() const { return this->specification.title; }
		inline unsigned int GetWidth() const { return this->specification.width; }
		inline unsigned int GetHeight() const { return this->specification.height; }

		inline void SetWindowSize(unsigned int new_width, unsigned int new_height) { this->specification.width = new_width, this->specification.height = new_height; }
		inline void SetWindowMode(VideoMode new_videomode) { this->specification.videomode = new_videomode; }
		inline void SetTitle(const std::string& new_title) { this->specification.title = new_title; }

		inline bool isMinimized() const { return glfwGetWindowAttrib(this->glfwWindow, GLFW_ICONIFIED); }
	private:
		WindowSpecification specification;
		GLFWwindow* glfwWindow = nullptr;
	};
}

