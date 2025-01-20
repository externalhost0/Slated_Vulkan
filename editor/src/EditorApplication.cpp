//
// Created by Hayden Rivas on 1/8/25.
//

// external headers
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <chrono>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Slate Headers
#include "Slate/VK/PipelineBuilder.h"
#include "Slate/VK/vkext.h"
#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkutil.h"
#include <Slate/Debug.h>
#include <Slate/Window.h>
#include <Slate/Time.h>

// editor headers
#include "EditorApplication.h"
#include "Primitives.h"

namespace Slate {
	// forward declaration
	void Aquire(VulkanEngine& engine);
	void Present(VulkanEngine& engine);

	static void glfw_error_callback(int error, const char* description) {
		fprintf(stderr, "[GLFW] Error %d: %s\n", error, description);
	}
	void standardglfw() {
		#if defined(SLATE_DEBUG)
		glfwSetErrorCallback(glfw_error_callback);
		#endif
		glfwInit();
	}

	vktypes::GPUMeshBuffers rectangle;

	void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
		return;
	}
	void OnResize(GLFWwindow* window, int width, int height) {
		return;
	}

	void EditorApplication::Initialize() {
		standardglfw();


		// build window
		WindowSpecification win_spec = {
				.IsResizeable = true,
				.WindowTitle = "Vulkan Test",
				.VideoMode = VIDEO_MODE::WINDOWED,
		};
		Ref<Window> _editorWindow = CreateRef<Window>(win_spec);
		_editorWindow->Build();
		// inject the window into our app systems that need them
		GetWindowSystem().InjectWindow(_editorWindow);
		GetInputSystem().InjectWindow(_editorWindow);


		glfwSetKeyCallback(_editorWindow->GetNativeWindow(), OnKey);
		glfwSetWindowSizeCallback(_editorWindow->GetNativeWindow(), OnResize);


		VulkanInstanceInfo editorInfo {
				.app_name = "Slate Editor",
				.app_version = {0, 0, 1},
				.engine_name = "Slate Engine",
				.engine_version = {0, 0, 1}
		};
		auto& engine = GetRenderSystem().GetEngine();
		{
			engine.CreateInstance(editorInfo);
			engine.CreateSurface(_editorWindow->GetNativeWindow());
			engine.CreateDevices();
			engine.CreateAllocator();
			engine.CreateQueues();
			engine.CreateSwapchain();
			engine.CreateCommands();
			engine.CreateSyncStructures();
			vkext::LoadExtensionFunctions(engine._vkDevice);
		}




		VkPushConstantRange bufferRange = {};
		bufferRange.offset = 0;
		bufferRange.size = sizeof(vktypes::GPUDrawPushConstants);
		bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinfo::CreatePipelineLayoutInfo();
		pipeline_layout_info.pPushConstantRanges = &bufferRange;
		pipeline_layout_info.pushConstantRangeCount = 1;

		VK_CHECK(vkCreatePipelineLayout(engine._vkDevice, &pipeline_layout_info, nullptr, &engine._vkMainPipelineLayout));

		VkShaderModule vertShader = vkutil::CreateShaderModule("shaders/compiled_shaders/standard.vert.spv", engine._vkDevice);
		VkShaderModule fragShader = vkutil::CreateShaderModule("shaders/compiled_shaders/standard.frag.spv", engine._vkDevice);

		PipelineBuilder pipelineBuilder;
		VkPipeline pipelineresult = pipelineBuilder
											.set_shaders(vertShader, fragShader)
											.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
											.set_polygon_mode(VK_POLYGON_MODE_FILL)
											.set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
											.set_multisampling_none()
											.disable_blending()
											.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL)
											.set_color_attachment_format(engine._drawImage.imageFormat)
											.set_depth_format(engine._depthImage.imageFormat)
									  .build(engine._vkDevice, engine._vkMainPipelineLayout);
		engine._vkMainPipeline = pipelineresult;

		vkDestroyShaderModule(engine._vkDevice, fragShader, nullptr);
		vkDestroyShaderModule(engine._vkDevice, vertShader, nullptr);

		rectangle = engine.UploadMesh(Primitives::Vertices, Primitives::Indices);


		// gui setup
		_editorGui.InjectWindow(_editorWindow);

		_camera = CreateRef<ViewportCamera>();
		_editorGui.InjectViewportCamera(_camera);

		_editorGui.OnAttach(engine);
		// do this at the end
		ImGui_ImplGlfw_InstallCallbacks(GetWindowSystem().GetMainWindow()->GetNativeWindow());
	}

	void EditorApplication::Loop() {
		glfwPollEvents();
		auto nativeWindow = GetWindowSystem().GetMainWindow()->GetNativeWindow();
		auto& engine = GetRenderSystem().GetEngine();

		if (GetInputSystem().IsKeyPressed(GLFW_KEY_Q)) {
			glfwSetWindowShouldClose(nativeWindow, true);
		}
		if (glfwGetWindowAttrib(nativeWindow, GLFW_ICONIFIED) == GLFW_TRUE) isMinimized = true;
		else isMinimized = false;
		// throttling on minimize
		if (isMinimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			return;
		}
		if (engine.resizeRequested) engine.ResizeSwapchain(nativeWindow);
		_editorGui.OnUpdate(GetInputSystem(), engine);

		{
			Aquire(engine);

			VkCommandBufferBeginInfo cmdbegininfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			VK_CHECK(vkBeginCommandBuffer(engine.getCurrentFrameData()._commandBuffer, &cmdbegininfo));

			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			// clearing of background
			{
				VkClearColorValue clearValue;
				clearValue = { { 0.0f, 0.4f, 0.8f, 1.0f } };
				VkImageSubresourceRange clearRange = vkinfo::CreateImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
				vkCmdClearColorImage(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
			}

			// our main pass includes a color and depth image
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

			// drawing test of objects
			{
				VkRenderingAttachmentInfo colorAttachment = vkinfo::CreateAttachmentInfo(engine._drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				VkRenderingAttachmentInfo depthAttachment = vkinfo::CreateDepthAttachmentInfo(engine._depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

				VkExtent2D screenExtent = engine._drawImage.GetExtent2D();

				VkRenderingInfo renderInfo = vkinfo::CreateRenderingInfo(screenExtent, &colorAttachment, &depthAttachment);
				vkext::vkCmdBeginRendering(engine.getCurrentFrameData()._commandBuffer, &renderInfo);
				vkutil::SetViewport(engine.getCurrentFrameData()._commandBuffer, screenExtent);
				vkutil::SetScissor(engine.getCurrentFrameData()._commandBuffer, screenExtent);

				// bind and draw index
				{
					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._vkMainPipeline);
					vktypes::GPUDrawPushConstants push_constants = {};

					// translation test
					auto model = glm::mat4(1.f);
					model = glm::translate(model, engine.transformtest);



					_camera->ProcessKeys(GetInputSystem());
					_camera->Update();

					push_constants.worldMatrix = _camera->GetProjectionMatrix() * _camera->GetViewMatrix() * model;
					push_constants.vertexBuffer = rectangle.vertexBufferAddress;

					vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._vkMainPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vktypes::GPUDrawPushConstants), &push_constants);

					vkCmdBindIndexBuffer(engine.getCurrentFrameData()._commandBuffer, rectangle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(engine.getCurrentFrameData()._commandBuffer, 6, 1, 0, 0, 0);
				}
				vkext::vkCmdEndRendering(engine.getCurrentFrameData()._commandBuffer);
			}

//			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			// blit
			{
//				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//				vkutil::BlitImageToImage(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, engine._vkSwapchainImages[engine._imageIndex], engine._drawImage.GetExtent2D(), engine._vkSwapchianExtent);
//				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}
			// imgui drawing
			{
				_editorGui.Render(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImageViews[engine._imageIndex], engine._vkSwapchianExtent);
			}

			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
//			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			VK_CHECK(vkEndCommandBuffer(engine.getCurrentFrameData()._commandBuffer));

			Present(engine);
			engine._frameNum++;
		}

		if (glfwWindowShouldClose(nativeWindow)) {
			continue_Loop = false;
		}
	}

	void EditorApplication::Shutdown() {
		auto& engine = GetRenderSystem().GetEngine();
		vkDeviceWaitIdle(engine._vkDevice); // required
		{
			_editorGui.OnDetach();
		}
	}

	void Aquire(VulkanEngine& engine) {
		VK_CHECK(vkWaitForFences(engine._vkDevice, 1, &engine.getCurrentFrameData()._renderFence, VK_TRUE, UINT64_MAX));
		VK_CHECK(vkResetFences(engine._vkDevice, 1, &engine.getCurrentFrameData()._renderFence));

		VkResult aquireResult = vkAcquireNextImageKHR(
				engine._vkDevice,
				engine._vkSwapchain,
				UINT64_MAX,
				engine.getCurrentFrameData()._swapchainSemaphore,
				nullptr,
				&engine._imageIndex);
		if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR || aquireResult == VK_SUBOPTIMAL_KHR) {
			engine.resizeRequested = true;
			if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
		VK_CHECK(vkResetCommandPool(engine._vkDevice, engine.getCurrentFrameData()._commandPool, 0));
	}

	void Present(VulkanEngine& engine) {

		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(engine.getCurrentFrameData()._commandBuffer);
		VkSemaphoreSubmitInfo waitInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, engine.getCurrentFrameData()._swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = vkinfo::CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, engine.getCurrentFrameData()._renderSemaphore);

		VkSubmitInfo2 submitInfo = vkinfo::CreateSubmitInfo(&cmdinfo, &signalInfo, &waitInfo);
		vkext::vkQueueSubmit2(engine._vkGraphicsQueue, 1, &submitInfo, engine.getCurrentFrameData()._renderFence);

		VkPresentInfoKHR presentinfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = nullptr };
		presentinfo.pSwapchains = &engine._vkSwapchain;
		presentinfo.swapchainCount = 1;
		presentinfo.pWaitSemaphores = &engine.getCurrentFrameData()._renderSemaphore;
		presentinfo.waitSemaphoreCount = 1;
		presentinfo.pImageIndices = &engine._imageIndex;

		VkResult presentResult = vkQueuePresentKHR(engine._vkGraphicsQueue, &presentinfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
			engine.resizeRequested = true;
			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) return;
		}
	}
}