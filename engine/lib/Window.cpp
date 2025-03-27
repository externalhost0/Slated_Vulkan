//
// Created by Hayden Rivas on 1/8/25.
//
#include <GLFW/glfw3.h>

#include "Slate/Window.h"
#include "Slate/Debug.h"

namespace Slate {
	void Window::Initialize() {
		// get some properties of user monitor
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		// window hints
		{
			// static hints
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

			// variable hints
			glfwWindowHint(GLFW_RESIZABLE, this->specification.resizeable);

		}

		// video mode setting logic
		int w = mode->width;
		int h = mode->height;
		if (this->specification.videomode == VideoMode::BORDERLESS_FULLSCREEN) {
			monitor = nullptr;
		} else if (this->specification.videomode == VideoMode::WINDOWED) {
			monitor = nullptr;
			w = static_cast<int>(this->specification.width);
			h = static_cast<int>(this->specification.height);
		}
		this->glfwWindow = glfwCreateWindow(w, h, this->specification.title.c_str(), monitor, nullptr);
	}
	void Window::Shutdown() {
		glfwDestroyWindow(this->glfwWindow);
	}
}