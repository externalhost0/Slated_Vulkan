//
// Created by Hayden Rivas on 1/8/25.
//
#include "EditorApplication.h"


#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <Slate/Window.h>

namespace Slate {

	vkb::Instance vkb_instance;

	void EditorApplication::Initialize() {
		// windowing
		WindowSpecification spec1;
		spec1.WindowTitle = "Vulkan Test";
		spec1.VideoMode = VIDEO_MODE::BORDERLESS_FULLSCREEN;
		spec1.IsResizeable = true;
		spec1.VSyncEnabled = false;

		Window window1(spec1);
		window1.Build();
		SystemLocator::Get<WindowManager>().SetMainWindow(window1);

		//vkb setup
		vkb::InstanceBuilder builder;
		auto inst_ret = builder.set_app_name("Untitled App")
				.request_validation_layers()
				.use_default_debug_messenger()
				.require_api_version(1, 2, 0)
				.build();
		if (!inst_ret)
			EXPECT(false, "Failed to create Vulkan instance. Error: %s", inst_ret.error().message().c_str())

		vkb_instance = inst_ret.value();



		VkSurfaceKHR surface = nullptr;
		if (glfwCreateWindowSurface(vkb_instance.instance, window1.GetNativeWindow(), nullptr, &surface) != VK_SUCCESS)
			EXPECT(false, "Window Surface Creation Failed!")



		vkb::PhysicalDeviceSelector selector{vkb_instance};
		auto phys_ret = selector.set_surface(surface)
								.set_minimum_version(1, 2)
								.require_dedicated_transfer_queue()
								.select();
		if (!phys_ret)
			EXPECT(false, "Failed to select Vulkan Physical Device. Error: %s", phys_ret.error().message().c_str())

	}
	void EditorApplication::Loop() {
		auto nativeWindow = SystemLocator::Get<WindowManager>().GetWindow()->GetNativeWindow();
		glfwPollEvents();
		{



		}
		glfwSwapBuffers(nativeWindow);
		if (glfwWindowShouldClose(nativeWindow)) {
			shouldStopLoop = true;
		}
	}
	void EditorApplication::Shutdown() {

		vkb::destroy_instance(vkb_instance);

		SystemLocator::Get<WindowManager>().GetWindow()->Destroy();
		glfwTerminate();
	}
}