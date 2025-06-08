//
// Created by Hayden Rivas on 1/7/25.
//

#include "Slate/IApplication.h"
#include "Slate/Common/HelperMacros.h"
#include "Slate/Filesystem.h"

namespace Slate {
//	void window_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
//		app_cast->onWindowKey();
//	}
//	void window_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
//		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
//		app_cast->onWindowMouseScroll();
//	}
//	void window_mousebutton_callback(GLFWwindow* window, int button, int action, int mods) {
//		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
//		app_cast->onWindowMouseButton();
//	}
	void window_resize_callback(GLFWwindow* window, int width, int height) {
		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
		app_cast->onWindowResize(width, height);
	}
	void window_move_callback(GLFWwindow* window, int xpos, int ypos) {
		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
		app_cast->onWindowMove(xpos, ypos);
	}
	void window_close_callback(GLFWwindow* window) {
		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
		app_cast->onWindowClose();
	}
	void window_focus_callback(GLFWwindow* window, int focused) {
		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
		app_cast->onWindowFocus();
	}
	void window_minimize_callback(GLFWwindow* window, int iconified) {
		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
		app_cast->onWindowMinimize();
	}
	void window_maximize_callback(GLFWwindow* window, int maximized) {
		auto app_cast = static_cast<IApplication*>(glfwGetWindowUserPointer(window));
		app_cast->onWindowMaximize();
	}

	void IApplication::start() {
		int result = glfwInit();
		ASSERT_MSG(result = GLFW_TRUE, "[GLFW] Failed to initialize GLFW.");
		this->onInitialize();
		VulkanInstanceInfo vk_info = {
				.app_name = "Slate Example App",
				.app_version = {0, 0, 1},
				.engine_name = "Slate Engine",
				.engine_version = {0, 0, 1}
		};
		_gx.create(vk_info, _window.getGLFWWindow());
	}
	void IApplication::loop() {
		glfwPollEvents();
		{
			this->onTick();
			if (_gx.isSwapchainDirty()) {
				int w, h;
				glfwGetFramebufferSize(_window.getGLFWWindow(), &w, &h);
				_gx.resizeSwapchain(w, h);
				this->onSwapchainResize();
			}
			this->onRender();
		}
		if (glfwWindowShouldClose(_window.getGLFWWindow())) { callStop(); }
		_apptime.update();
	}
	void IApplication::stop() {
		this->onShutdown();
		_window.destroy();
		_gx.destroy();
		glfwTerminate();
	}
	void IApplication::installAppCallbacksToWindow(GLFWwindow* window) {
		glfwSetWindowUserPointer(window, this);

		glfwSetWindowSizeCallback(window, window_resize_callback);
		glfwSetWindowCloseCallback(window, window_close_callback);
		glfwSetWindowFocusCallback(window, window_focus_callback);
		glfwSetWindowPosCallback(window, window_move_callback);
		glfwSetWindowIconifyCallback(window, window_minimize_callback);
		glfwSetWindowMaximizeCallback(window, window_maximize_callback);
	}
}