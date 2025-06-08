//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once
#include "Slate/Common/Invalids.h"

#include <GLFW/glfw3.h>
#include <string>
#include <utility>

namespace Slate {
	enum class VideoMode : char {
		Windowed = 0,
		BorderlessFullscreen,
		Fullscreen,
	};
	// lets design important classes with [type]specification
	struct WindowSpec
	{
		VideoMode videomode = VideoMode::Windowed;
		std::string title = "Untitled Window";
		bool resizeable = false;
		uint16_t width = 1280, height = 720;
	};

	class Window final {
	public:
		Window() = default;
		~Window() { destroy(); };

		void create(const WindowSpec& new_spec);
		void destroy();
	public:
		inline GLFWwindow* getGLFWWindow()  { return this->glfwWindow; }

		inline const std::string& getTitle() const { return this->spec.title; }
		inline uint16_t getWidth() const { return this->spec.width; }
		inline uint16_t getHeight() const { return this->spec.height; }

		void setWindowMode(VideoMode new_videomode);
		void setWindowSize(uint16_t new_width, uint16_t new_height);
		void setTitle(const std::string& new_title);

		inline bool isMaximized() const { return glfwGetWindowAttrib(this->glfwWindow, GLFW_MAXIMIZED); }
		inline bool isMinimized() const { return glfwGetWindowAttrib(this->glfwWindow, GLFW_ICONIFIED); }
		inline bool isFocused() const { return glfwGetWindowAttrib(this->glfwWindow, GLFW_FOCUSED); }
		inline bool isHovered() const { return glfwGetWindowAttrib(this->glfwWindow, GLFW_HOVERED); }
	private:
		WindowSpec spec;
		GLFWwindow* glfwWindow = nullptr;
	};
}

