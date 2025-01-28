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
#include <Slate/PipelineBuilder.h>
#include <Slate/VK/vkext.h>
#include <Slate/VK/vkinfo.h>
#include <Slate/VK/vkutil.h>
#include <Slate/Debug.h>
#include <Slate/Window.h>
#include <Slate/Entity.h>
#include <Slate/SceneTemplates.h>
#include <Slate/Components.h>

// editor headers
#include "EditorApplication.h"
#include "Slate/Primitives.h"

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

		_globals.window.SetSpecification(win_spec);
		_globals.window.Build();
		InputSystem::InjectWindow(_globals.window.GetNativeWindow());


		glfwSetKeyCallback(_globals.window.GetNativeWindow(), OnKey);
		glfwSetWindowSizeCallback(_globals.window.GetNativeWindow(), OnResize);


		VulkanInstanceInfo editorInfo {
				.app_name = "Slate Editor",
				.app_version = {0, 0, 1},
				.engine_name = "Slate Engine",
				.engine_version = {0, 0, 1}
		};
		auto& engine = _globals.engine;
		{
			engine.CreateInstance(editorInfo);
			engine.CreateSurface(_globals.window.GetNativeWindow());
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
		VK_CHECK(vkCreatePipelineLayout(engine._vkDevice, &pipeline_layout_info, nullptr, &engine._gridPipelineLayout));

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
											.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
											.set_color_attachment_format(engine._colorImage.imageFormat)
											.set_depth_format(engine._depthStencilImage.imageFormat)
									  .build(engine._vkDevice, engine._vkMainPipelineLayout);
		engine._standardPipeline = pipelineresult;

		VkPipeline pipelineresult2 = pipelineBuilder
									   .set_shaders(vertShader, fragShader)
									   .set_input_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
									   .set_polygon_mode(VK_POLYGON_MODE_LINE)
									   .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
									   .set_multisampling_none()
									   .disable_blending()
									   .enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
									   .set_color_attachment_format(engine._colorImage.imageFormat)
									   .set_depth_format(engine._depthStencilImage.imageFormat)
								   .build(engine._vkDevice, engine._gridPipelineLayout);
		engine._gridPipeline = pipelineresult2;

		vkDestroyShaderModule(engine._vkDevice, fragShader, nullptr);
		vkDestroyShaderModule(engine._vkDevice, vertShader, nullptr);



		// gui setup
		_globals.editorGui.engine = &_globals.engine;
		_globals.editorGui.pCamera = &_globals.camera;
		_globals.editorGui.pActiveContext = &_globals.activeContxt;
		_globals.editorGui.OnAttach(_globals.window.GetNativeWindow());
		{
			Scene& scene  = _globals.activeContxt.scene;
			vktypes::MeshData quadData = engine.CreateMesh(Primitives::quadVertices, Primitives::quadIndices);

			Entity cube1 =  scene.CreateEntity("Cubey");
			cube1.AddComponent<MeshComponent>(quadData);

			Entity cube2 = scene.CreateEntity("Cubey2");
			cube2.AddComponent<TransformComponent>(glm::vec3(1.f, 1.f, 1.f));
			cube2.AddComponent<MeshComponent>(quadData);
		}



		// do this at the end
		ImGui_ImplGlfw_InstallCallbacks(_globals.window.GetNativeWindow());
	}

	void EditorApplication::Loop() {
		glfwPollEvents();
		auto nativeWindow = _globals.window.GetNativeWindow();
		auto& engine = _globals.engine;

		if (InputSystem::IsKeyPressed(GLFW_KEY_Q)) { glfwSetWindowShouldClose(nativeWindow, true); }
		if (glfwGetWindowAttrib(nativeWindow, GLFW_ICONIFIED) == GLFW_TRUE) isMinimized = true;
		else isMinimized = false;
		// throttling on minimize
		if (isMinimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));
			return;
		}
		if (engine.resizeRequested) engine.OnResizeSwapchain(nativeWindow);

		ViewportCamera& camera = _globals.camera;
		camera.ProcessKeys();
		camera.Update();


		{
			engine.Aquire();

			VkCommandBufferBeginInfo cmdbegininfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			VK_CHECK(vkBeginCommandBuffer(engine.getCurrentFrameData()._commandBuffer, &cmdbegininfo));


			// our main pass includes a color and depth image
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._depthStencilImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			// drawing test of objects
			{
				VkRenderingAttachmentInfo colorAttachment = vkinfo::CreateAttachmentInfo(engine._colorImage.imageView, &engine.clearColorValue, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				VkClearDepthStencilValue clearDepthStencil = { 1.f, 0 };
				VkRenderingAttachmentInfo depthAttachment = vkinfo::CreateDepthStencilAttachmentInfo(engine._depthStencilImage.imageView, &clearDepthStencil, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

				VkExtent2D screenExtent = engine._colorImage.GetExtent2D();
				VkRenderingInfo renderInfo = vkinfo::CreateRenderingInfo(screenExtent, &colorAttachment, &depthAttachment);
				vkext::vkCmdBeginRendering(engine.getCurrentFrameData()._commandBuffer, &renderInfo);
				vkutil::SetViewport(engine.getCurrentFrameData()._commandBuffer, screenExtent);
				vkutil::SetScissor(engine.getCurrentFrameData()._commandBuffer, screenExtent);

				// draw some editor stuff
				{
					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._gridPipeline);
					_globals.editorGui.OnUpdate();
				}

				// bind and draw index
				{
					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._standardPipeline);
					for (Entity entity : _globals.activeContxt.scene.GetAllEntitiesWith<MeshComponent>()) {
						// require that the mesh has valid info
						if (!entity.GetComponent<MeshComponent>().data.constants.vertexBufferAddress) continue;
						// transform application
						{
							auto model = glm::mat4(1.0f);
							auto entityTC = entity.GetComponent<TransformComponent>();
							model = glm::translate(model, entityTC.Position);
							model = model * glm::mat4_cast(entityTC.Rotation);
							model = glm::scale(model, entityTC.Scale);

							// mesh update
							{
								auto entityMC = entity.GetComponent<MeshComponent>();
								entityMC.data.constants.renderMatrix = _globals.camera.GetProjectionMatrix() * _globals.camera.GetViewMatrix() * model;
								engine.Draw(entityMC.data, engine._vkMainPipelineLayout);
							}
						}
					}
				}
				vkext::vkCmdEndRendering(engine.getCurrentFrameData()._commandBuffer);
			}
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//			vkutil::BlitImageToImage(engine.getCurrentFrameData()._commandBuffer, engine._colorImage.image, engine._vkSwapchainImages[engine._imageIndex], engine._colorImage.GetExtent2D(), engine._vkSwapchianExtent);
//			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			// imgui drawing
			_globals.editorGui.Render(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImageViews[engine._imageIndex], engine._vkSwapchianExtent);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			VK_CHECK(vkEndCommandBuffer(engine.getCurrentFrameData()._commandBuffer));

			engine.Present();
		}
		if (glfwWindowShouldClose(nativeWindow)) { continue_Loop = false; }
	}

	void EditorApplication::Shutdown() {
		vkDeviceWaitIdle(_globals.engine._vkDevice); // required
		{
			_globals.editorGui.OnDetach(_globals.engine._vkDevice);
		}
	}



}