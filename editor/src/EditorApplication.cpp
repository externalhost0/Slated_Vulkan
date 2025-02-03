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
#include <Slate/Primitives.h>

// editor headers
#include "EditorApplication.h"

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
			engine.CreateSwapchain();
			engine.CreateAllocator();

			engine.CreateQueues();
			engine.CreateCommands();
			engine.CreateSyncStructures();

			engine.CreateStandardSamplers();
			engine.CreateDefaultBuffers();
			engine.CreateDefaultImages();

			engine.CreateDescriptors();

			vkext::LoadExtensionFunctions(engine._vkDevice);
		}



		VkPushConstantRange bufferRange = {};
		bufferRange.offset = 0;
		bufferRange.size = sizeof(vktypes::GPUDrawPushConstants);
		bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinfo::CreatePipelineLayoutInfo();
		{
			pipeline_layout_info.pPushConstantRanges = &bufferRange;
			pipeline_layout_info.pushConstantRangeCount = 1;
		}
		{
			pipeline_layout_info.pSetLayouts = &engine._gpuDescriptorSetLayout;
			pipeline_layout_info.setLayoutCount = 1;
		}

		VK_CHECK(vkCreatePipelineLayout(engine._vkDevice, &pipeline_layout_info, nullptr, &engine._standardPipeline.layout));
		VK_CHECK(vkCreatePipelineLayout(engine._vkDevice, &pipeline_layout_info, nullptr, &engine._gridPipeline.layout));
		PipelineBuilder pipelineBuilder;

		VkShaderModule standardVertShader = vkutil::CreateShaderModule("standard.vert.spv", engine._vkDevice);
		VkShaderModule standardFragShader = vkutil::CreateShaderModule("standard.frag.spv", engine._vkDevice);

		VkFormat formats[] = { engine._colorMSAAImage.imageFormat, engine._entityImage.imageFormat };

		engine._standardPipeline.pipeline = pipelineBuilder
										   .set_shaders(standardVertShader, standardFragShader)
										   .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
										   .set_polygon_mode(VK_POLYGON_MODE_FILL)
										   .set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
										   .set_multisampling_mode(VK_SAMPLE_COUNT_4_BIT)
										   .disable_blending()
										   .enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
										   .set_color_attachment_format(std::span(formats))
										   .set_depth_format(engine._depthStencilMSAAImage.imageFormat)
									  .build(engine._vkDevice, engine._standardPipeline.layout);
		vkDestroyShaderModule(engine._vkDevice, standardVertShader, nullptr);
		vkDestroyShaderModule(engine._vkDevice, standardFragShader, nullptr);

		VkShaderModule colorVertShader = vkutil::CreateShaderModule("editor_primitives.vert.spv", engine._vkDevice);
		VkShaderModule colorFragShader = vkutil::CreateShaderModule("editor_primitives.frag.spv", engine._vkDevice);

		engine._gridPipeline.pipeline = pipelineBuilder
									   .set_shaders(colorVertShader, colorFragShader)
									   .set_input_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
									   .set_polygon_mode(VK_POLYGON_MODE_LINE)
									   .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
									   .set_multisampling_mode(VK_SAMPLE_COUNT_4_BIT)
									   .enable_blending_additive()
									   .enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
									   .set_color_attachment_format(std::span(formats))
									   .set_depth_format(engine._depthStencilMSAAImage.imageFormat)
								   .build(engine._vkDevice, engine._gridPipeline.layout);
		vkDestroyShaderModule(engine._vkDevice, colorVertShader, nullptr);
		vkDestroyShaderModule(engine._vkDevice, colorFragShader, nullptr);

		// gui setup
		_globals.editorGui.engine = &_globals.engine;
		_globals.editorGui.pCamera = &_globals.camera;
		_globals.editorGui.pActiveContext = &_globals.activeContxt;
		_globals.editorGui.OnAttach(_globals.window.GetNativeWindow());
		{
			Scene& scene  = _globals.activeContxt.scene;

			vktypes::MeshData quadData = engine.CreateMesh(Primitives::quadVertices, Primitives::quadIndices);
			vktypes::MeshData planeData = engine.CreateMesh(Primitives::planeVertices, Primitives::quadIndices);
			vktypes::MeshData cubeData = engine.CreateMesh(Primitives::cubeVertices, Primitives::cubeIndices);


			Entity cube1 =  scene.CreateEntity("Cubey");
			cube1.AddComponent<MeshComponent>(quadData);

			Entity cube2 = scene.CreateEntity("Cubey2");
			cube2.AddComponent<TransformComponent>(glm::vec3{1.f, 1.f, 1.f});
			cube2.AddComponent<MeshComponent>(quadData);

			Entity standardPlane = scene.CreateEntity("Standard Plane");
			standardPlane.AddComponent<MeshComponent>(planeData);
			standardPlane.GetComponent<TransformComponent>().Scale = { 10.f, 1.f, 10.f };

			Entity defaultCube = scene.CreateEntity("Default Cube");
			defaultCube.AddComponent<TransformComponent>(glm::vec3{-5.f, 1.f, -1.f});
			defaultCube.AddComponent<MeshComponent>(cubeData);
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
		if (engine.resizeRequested) engine.OnResizeWindow(nativeWindow);

		ViewportCamera& camera = _globals.camera;
		camera.Update();
		_globals.editorGui.OnUpdate(); // idk where to put this


		{
			engine.Aquire();

			VkCommandBufferBeginInfo cmdbegininfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			vkBeginCommandBuffer(engine.getCurrentFrameData()._commandBuffer, &cmdbegininfo);

			// our main pass includes a color and depth image
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorMSAAImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._depthStencilMSAAImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._entityMSAAImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._entityImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			// drawing test of objects
			{
				VkClearColorValue entityClear = { -1, 0, 0, 0 };
				VkRenderingAttachmentInfo geometry_entity_attachment = vkinfo::CreateColorAttachmentInfo(engine._entityMSAAImage.imageView, &entityClear, engine._entityImage.imageView, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
				VkRenderingAttachmentInfo geometry_color_attachment = vkinfo::CreateColorAttachmentInfo(engine._colorMSAAImage.imageView, &engine.clearColorValue, engine._drawImage.imageView);
				VkClearDepthStencilValue depthClear = { 1.f, 0 };
				VkRenderingAttachmentInfo geometry_depth_attachment = vkinfo::CreateDepthStencilAttachmentInfo(engine._depthStencilMSAAImage.imageView, &depthClear);

				VkExtent2D renderExtent = engine._viewportImage.GetExtent2D();

				VkRenderingAttachmentInfo color_attachments[] = { geometry_color_attachment, geometry_entity_attachment};
				VkRenderingInfo geometry_render_info = vkinfo::CreateRenderingInfo(renderExtent, std::span(color_attachments), &geometry_depth_attachment);
				vkext::vkCmdBeginRendering(engine.getCurrentFrameData()._commandBuffer, &geometry_render_info);

				vkutil::SetViewport(engine.getCurrentFrameData()._commandBuffer, renderExtent);
				vkutil::SetScissor(engine.getCurrentFrameData()._commandBuffer, renderExtent);

				vktypes::GPUSceneData sceneData {
						.projectionMatrix = camera.GetProjectionMatrix(),
						.viewMatrix = camera.GetViewMatrix()
				};
				engine.UpdateDescriptorSets(sceneData);

				// draw some editor stuff
				{
					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._gridPipeline.pipeline);
					vkCmdBindDescriptorSets(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._gridPipeline.layout, 0, 1, &engine._gpuDescriptorSet, 0, nullptr);
					if (_globals.editorGui.gridIsEnabled) {
						_globals.editorGui.gridmesh.constants.modelMatrix = glm::translate(glm::mat4(1), {0.f, -0.004f, 0.f});
						engine.DrawMeshData(_globals.editorGui.gridmesh, engine._gridPipeline.layout);
					}

				}

				// bind and draw index
				{
					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._standardPipeline.pipeline);
					vkCmdBindDescriptorSets(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._standardPipeline.layout, 0, 1, &engine._gpuDescriptorSet, 0, nullptr);

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
								entityMC.data.constants.modelMatrix = model;
								entityMC.data.constants.id = static_cast<uint32_t>(entity.GetHandle());
								engine.DrawMeshData(entityMC.data, engine._standardPipeline.layout);
							}
						}
					}
				}
				vkext::vkCmdEndRendering(engine.getCurrentFrameData()._commandBuffer);



				// copy the resolved image onto the viewport image that is to be shown inside imgui
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				{
					vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._viewportImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
					vkutil::BlitImageToImage(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, engine._viewportImage.image, engine._drawImage.GetExtent2D(), engine._viewportImage.GetExtent2D());
					vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._viewportImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

				// render ui
				{
					VkRenderingAttachmentInfo ui_color_attachment = vkinfo::CreateColorAttachmentInfo(engine._drawImage.imageView, nullptr);
					VkRenderingInfo ui_rendering_info = vkinfo::CreateRenderingInfo(engine._drawImage.GetExtent2D(), &ui_color_attachment, nullptr);
					vkext::vkCmdBeginRendering(engine.getCurrentFrameData()._commandBuffer, &ui_rendering_info);
					_globals.editorGui.Render();
					vkext::vkCmdEndRendering(engine.getCurrentFrameData()._commandBuffer);
				}
			}
			// present by blitting to swapchain
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			{
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				vkutil::BlitImageToImage(engine.getCurrentFrameData()._commandBuffer, engine._drawImage.image, engine._vkSwapchainImages[engine._imageIndex], engine._drawImage.GetExtent2D(), engine._vkSwapchianExtent);
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			}
			VK_CHECK(vkEndCommandBuffer(engine.getCurrentFrameData()._commandBuffer));


			engine.Present();
		}
		if (glfwWindowShouldClose(nativeWindow)) { continue_Loop = false; }
	}

	void EditorApplication::Shutdown() {
		vkDeviceWaitIdle(_globals.engine._vkDevice); // required
		{
			_globals.editorGui.OnDetach();
		}
	}
}