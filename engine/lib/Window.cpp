//
// Created by Hayden Rivas on 1/8/25.
//
#include <GLFW/glfw3.h>
#include "Slate/Window.h"

namespace Slate {

	void WindowSystem::Initialize() {
		_isInitialized = true;
	}
	void WindowSystem::Shutdown() {
		EXPECT(_isInitialized, "WindowSystem has not been initialized.")
		// glfw cleanup
		glfwTerminate();
	}

	void Window::Build() {
		EXPECT(glfwVulkanSupported(), "[GLFW] Vulkan Not Supported!")

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
			glfwWindowHint(GLFW_RESIZABLE, m_Spec.IsResizeable);
		}


		// video mode setting logic
		int width = mode->width;
		int height = mode->height;
		if (m_Spec.VideoMode == VIDEO_MODE::BORDERLESS_FULLSCREEN) {
			monitor = nullptr;
		} else if (m_Spec.VideoMode == VIDEO_MODE::WINDOWED) {
			monitor = nullptr;
			width = static_cast<int>(m_Spec.WindowWidth);
			height = static_cast<int>(m_Spec.WindowHeight);
		}

		m_NativeWindow = glfwCreateWindow(width, height, m_Spec.WindowTitle.c_str(), monitor, nullptr);
	}
	void Window::Destroy() {
		glfwDestroyWindow(m_NativeWindow);
	}
}