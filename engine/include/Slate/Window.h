//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <utility>


namespace Slate {
	enum class VideoMode : unsigned char {
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

		void Initialize();
		void Shutdown();
	public:
		GLFWwindow* GetGlfwWindow()  { return this->glfwWindow; }
		std::string GetTitle() const { return this->specification.title; }
		unsigned int GetWidth() const { return this->specification.width; }
		unsigned int GetHeight() const { return this->specification.height; }

		void SetWindowSize(unsigned int new_width, unsigned int new_height) { this->specification.width = new_width, this->specification.height = new_height; }
		void SetWindowMode(VideoMode new_videomode) { this->specification.videomode = new_videomode; }
		void SetTitle(const std::string& new_title) { this->specification.title = new_title; }

		bool isMinimized() const { return glfwGetWindowAttrib(this->glfwWindow, GLFW_ICONIFIED); }
	private:
		WindowSpecification specification;
		GLFWwindow* glfwWindow = nullptr;
	};
}

