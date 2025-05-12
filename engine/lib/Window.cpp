//
// Created by Hayden Rivas on 1/8/25.
//
#include <GLFW/glfw3.h>

#include "Slate/Common/Debug.h"
#include "Slate/Window.h"

namespace Slate {
	void Window::create(const WindowSpec& new_spec) {
		this->spec = new_spec;
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
			glfwWindowHint(GLFW_RESIZABLE, this->spec.resizeable);

		}
		// video mode setting logic
		int w = mode->width;
		int h = mode->height;
		if (this->spec.videomode == VideoMode::BorderlessFullscreen) {
			monitor = nullptr;
		} else if (this->spec.videomode == VideoMode::Windowed) {
			monitor = nullptr;
			w = static_cast<int>(this->spec.width);
			h = static_cast<int>(this->spec.height);
		}
		this->glfwWindow = glfwCreateWindow(w, h, this->spec.title.c_str(), monitor, nullptr);
	}
	void Window::destroy() {
		this->spec = {};
		glfwDestroyWindow(this->glfwWindow);
	}

	void Window::setWindowMode(VideoMode new_videomode) {
		this->spec.videomode = new_videomode;
		switch (new_videomode) {
			case VideoMode::Windowed: {
				glfwSetWindowMonitor(this->glfwWindow, nullptr, 0, 0, spec.width, spec.height, 0);
				break;
			}
			case VideoMode::BorderlessFullscreen: {
				GLFWmonitor* monitor = glfwGetWindowMonitor(this->glfwWindow);
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				glfwSetWindowAttrib(this->glfwWindow, GLFW_DECORATED, GLFW_FALSE);
				glfwSetWindowMonitor(this->glfwWindow, nullptr, 0, 0, mode->width, mode->height, 0);
				break;
			}
			case VideoMode::Fullscreen: {
				GLFWmonitor* monitor = glfwGetWindowMonitor(this->glfwWindow);
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(this->glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				break;
			}
		}
	}
	void Window::setWindowSize(uint16_t new_width, uint16_t new_height) {
		this->spec.width = new_width, this->spec.height = new_height;
		glfwSetWindowSize(this->glfwWindow, new_width, new_height);
	}
	void Window::setTitle(const std::string& new_title) {
		this->spec.title = new_title;
		glfwSetWindowTitle(this->glfwWindow, new_title.c_str());
	}
}