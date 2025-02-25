//
// Created by Hayden Rivas on 1/8/25.
//

// external headers

#include <imgui_impl_vulkan.h>
#include <Slate/VulkanEngine.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#include <span>
#include <chrono>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Slate Headers
#include <Slate/PipelineBuilder.h>
#include <Slate/VK/vkinfo.h>
#include <Slate/VK/vkutil.h>
#include <Slate/Debug.h>
#include <Slate/Window.h>
#include <Slate/Entity.h>
#include <Slate/SceneTemplates.h>
#include <Slate/Components.h>
#include <Slate/Primitives.h>
#include <Slate/MeshGenerators.h>

// editor headers
#include "EditorApplication.h"

namespace Slate {
	// forward declaration
	void Aquire(VulkanEngine& engine);
	void Present(VulkanEngine& engine);

	static void glfw_error_callback(int error, const char* description) {
		fprintf(stderr, "[GLFW] Error %d: %s\n", error, description);
	}



	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto* pointer = static_cast<EditorApplication*>(glfwGetWindowUserPointer(window));
		pointer->App_OnKey(key, action, mods);
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		auto* pointer = static_cast<EditorApplication*>(glfwGetWindowUserPointer(window));
		pointer->App_OnMouseButton();
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}


	void EditorApplication::App_OnKey(int key, int action, int mods) {
		// quit
		if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(this->_globals.window.GetNativeWindow(), true);

		switch (this->_globals.editorGui._currenthovered) {
			case HoverWindow::ViewportWindow: {
				if (_globals.editorGui._isCameraControlActive) {

				} else {
					if (key == GLFW_KEY_W) {
						this->_globals.editorGui._guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
					}
					if (key == GLFW_KEY_E) {
						this->_globals.editorGui._guizmoOperation = ImGuizmo::OPERATION::ROTATE;
					}
					if (key == GLFW_KEY_R) {
						this->_globals.editorGui._guizmoOperation = ImGuizmo::OPERATION::SCALE;
					}
				}
				break;
			}
			case HoverWindow::PropertiesWindow: {

				break;
			}
			case HoverWindow::ScenePanel: {
				break;
			}
			case HoverWindow::AssetsPanel: {
				break;
			}
		}


	}
	void EditorApplication::App_OnMouseButton() {

	}
	void EditorApplication::App_OnMouseMove() {

	}
	void EditorApplication::App_OnWindowResize() {

	}
	glm::mat4 TransformToModelMatrix(TransformComponent& component, bool isScalable = true, bool isRotatable = true) {
		auto model = glm::mat4(1);
		model = glm::translate(model, component.Position);
		if (isRotatable) model = model * glm::mat4_cast(component.Rotation);
		if (isScalable) model = glm::scale(model, component.Scale);
		return model;
	}


	void EditorApplication::Initialize() {
		EXPECT(glfwInit() == GLFW_TRUE, "[GLFW] Init of GLFW failed!"); // we need a better place to do this
//		EXPECT(glfwVulkanSupported() == GLFW_TRUE, "[GLFW] Vulkan Not Supported!");
		#if defined(SLATE_DEBUG)
		glfwSetErrorCallback(glfw_error_callback);
		#endif


		// build window
		WindowSpecification win_spec = {
				.IsResizeable = true,
				.WindowTitle = "Vulkan Test",
				.VideoMode = VIDEO_MODE::WINDOWED,
		};
		_globals.window.SetSpecification(win_spec);
		_globals.window.Build();
		InputSystem::InjectWindow(_globals.window.GetNativeWindow());



		glfwSetWindowUserPointer(_globals.window.GetNativeWindow(), this);
		glfwSetKeyCallback(_globals.window.GetNativeWindow(), KeyCallback);
		glfwSetMouseButtonCallback(_globals.window.GetNativeWindow(), MouseButtonCallback);

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
			engine.CreateSwapchain();
			engine.CreateQueues();
			engine.CreateCommands();
			engine.CreateSyncStructures();
			engine.CreateFrameDescriptors();
			engine.CreateDefaultData(); // samplers as of now, other things that the engine should provide goes here
		}

		// editor meshes
		{
			std::vector<Vertex_Standard> vertices;

			vertices.clear();
			GenerateGrid(vertices, 100.f);
			this->gridmesh = engine.CreateMesh(vertices);

			vertices.clear();
			GenerateArrow2DMesh(vertices, 2.f, 1.f, 0.5f, 0.8f);
			this->arrowmesh = engine.CreateMesh(vertices);

			vertices.clear();
			GenerateSimpleSphere(vertices, 50);
			this->simplespheremesh = engine.CreateMesh(vertices);

			vertices.clear();
			GenerateSpot(vertices, 10);
			this->spotmesh = engine.CreateMesh(vertices);
		}

		engine.CreateDefaultImages();

		// icon loading
		lightbulb_image = engine.CreateImage("textures/icons/lightbulb.png", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);
		sun_image = engine.CreateImage("textures/icons/sun.png", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);
		spotlight_image = engine.CreateImage("textures/icons/lamp-ceiling.png", VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);

		engine._cameraDataBuffer = engine.CreateBuffer(sizeof(GPU::CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		engine._lightingDataBuffer = engine.CreateBuffer(sizeof(GPU::LightingUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		engine._colorBuffer = engine.CreateBuffer(sizeof(GPU::ExtraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		// top level descriptor
		{
			DescriptorLayoutBuilder dsbuilder;
			this->_perSceneDescLayout = dsbuilder
												 .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) // camera meshData
										 		 .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) // lighting meshData
											  .build(engine._vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		{
			DescriptorLayoutBuilder dsbuilder;
			this->_perShaderDescLayout = dsbuilder
												 .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
											 .build(engine._vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		{
			DescriptorLayoutBuilder dsbuilder;
			this->_perObjectDescLayout = dsbuilder
												 .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
											 .build(engine._vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}

		// this is req for now
		{
			DescriptorLayoutBuilder dsbuilder;
			engine._centralDescriptorSetLayout = dsbuilder
														 .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) // scene meshData
														 .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) // lighting meshData
														 .build(engine._vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		{
			DescriptorLayoutBuilder dsbuilder;
			engine._imageDescriptorSetLayout = dsbuilder
													   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) // scene meshData
													   .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) // texture meshData
													   .addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) // extra UBO
													   .build(engine._vkDevice, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		}



		// pipeline layout creation, prefaces pipeline creation
		{
			VkPushConstantRange bufferRange = {};
			bufferRange.offset = 0;
			bufferRange.size = sizeof(GPU::DrawPushConstants);
			bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkPipelineLayoutCreateInfo pipeline_layout_info = vkinfo::CreatePipelineLayoutInfo(&bufferRange, &engine._centralDescriptorSetLayout);
			VK_CHECK(vkCreatePipelineLayout(engine._vkDevice, &pipeline_layout_info, nullptr, &engine._standardPL));
			VkPipelineLayoutCreateInfo pipeline_layout_info_2 = vkinfo::CreatePipelineLayoutInfo(&bufferRange, &engine._imageDescriptorSetLayout);
			VK_CHECK(vkCreatePipelineLayout(engine._vkDevice, &pipeline_layout_info_2, nullptr, &engine._imagePL));
		}

		PipelineBuilder pipelineBuilder {};
		VkFormat formats[] = { engine._colorMSAAImage.imageFormat, engine._entityMSAAImage.imageFormat };

		// creating shader program should:
		// generate all required buffers
		// generate all bindings for pipeline
		// generate all push constants
		ShaderProgram standardProgram = vkutil::CreateShaderProgram(engine._vkDevice, "standard.slang.spv");


		engine._standardPipeline = pipelineBuilder
										   .set_program(standardProgram)
										   .set_default_topology_mode(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
										   .set_default_polygon_mode(VK_POLYGON_MODE_FILL)
										   .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
										   .set_multisampling_mode(VK_SAMPLE_COUNT_4_BIT)
										   .disable_blending()
										   .enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
										   .set_color_attachment_format(std::span(formats))
										   .set_depth_format(engine._depthStencilMSAAImage.imageFormat)
									   .build(engine._vkDevice, engine._standardPL);
		vkutil::DestroyShaderProgramModules(engine._vkDevice, standardProgram);


		ShaderProgram colorProgram = vkutil::CreateShaderProgram(engine._vkDevice, "editor_primitives.slang.spv");
		engine._plaincolorTrianglePipeline = pipelineBuilder
											 .set_program(colorProgram)
											 .set_default_topology_mode(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
											 .set_default_polygon_mode(VK_POLYGON_MODE_LINE)
											 .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
											 .set_multisampling_mode(VK_SAMPLE_COUNT_4_BIT)
											 .enable_blending_alphablend()
											 .enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
											 .set_color_attachment_format(std::span(formats))
											 .set_depth_format(engine._depthStencilMSAAImage.imageFormat)
										 .build(engine._vkDevice, engine._standardPL);
		engine._plaincolorLinePipeline = pipelineBuilder
											 .set_program(colorProgram)
											 .set_default_topology_mode(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP)
											 .set_default_polygon_mode(VK_POLYGON_MODE_LINE)
											 .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
											 .set_multisampling_mode(VK_SAMPLE_COUNT_4_BIT)
											 .enable_blending_alphablend()
											 .enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
											 .set_color_attachment_format(std::span(formats))
											 .set_depth_format(engine._depthStencilMSAAImage.imageFormat)
										 .build(engine._vkDevice, engine._standardPL);
		vkutil::DestroyShaderProgramModules(engine._vkDevice, colorProgram);


		ShaderProgram imageProgram = vkutil::CreateShaderProgram(engine._vkDevice, "editor_images.slang.spv");
		engine._imagePipeline = pipelineBuilder
										.set_program(imageProgram)
										.set_default_topology_mode(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
										.set_default_polygon_mode(VK_POLYGON_MODE_FILL)
										.set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
										.set_multisampling_mode(VK_SAMPLE_COUNT_4_BIT)
										.enable_blending_premultiplied()
										.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL)
										.set_color_attachment_format(std::span(formats))
										.set_depth_format(engine._depthStencilMSAAImage.imageFormat)
									.build(engine._vkDevice, engine._imagePL);
		vkutil::DestroyShaderProgramModules(engine._vkDevice, imageProgram);


		// gui setup, super scuffed
		_globals.editorGui.engine = &_globals.engine;
		_globals.editorGui.pCamera = &_globals.camera;
		_globals.editorGui.pActiveContext = &_globals.activeContxt;
		_globals.editorGui.OnAttach(_globals.window.GetNativeWindow());
		{
			Scene& scene  = _globals.activeContxt.scene;

			quadData = engine.CreateMesh(Primitives::quadVertices, Primitives::quadIndices);
			vktypes::MeshData planeData = engine.CreateMesh(Primitives::planeVertices, Primitives::quadIndices);
			vktypes::MeshData cubeData = engine.CreateMesh(Primitives::cubeVertices, Primitives::cubeIndices);

			std::vector<Vertex_Standard> vertices;
			std::vector<uint32_t> indices;
			GenerateSphere(vertices, indices, 1.f, 25, 25);
			vktypes::MeshData sphereData = engine.CreateMesh(vertices, indices);

			Entity cube1 =  scene.CreateEntity("Sphere");
			cube1.AddComponent<TransformComponent>(glm::vec3{-2.f, 4.f, 0.f});
			cube1.AddComponent<MeshComponent>(sphereData, standardProgram);

			Entity cube2 = scene.CreateEntity("Quad");
			cube2.AddComponent<TransformComponent>(glm::vec3{1.f, 1.f, 1.f});
			cube2.AddComponent<MeshComponent>(quadData, standardProgram);

			Entity standardPlane = scene.CreateEntity("Standard Plane");
			standardPlane.AddComponent<MeshComponent>(planeData, standardProgram);
			standardPlane.GetComponent<TransformComponent>().Scale = { 10.f, 1.f, 10.f };

			Entity defaultCube = scene.CreateEntity("Default Cube");
			defaultCube.AddComponent<TransformComponent>(glm::vec3{-5.f, 1.f, -1.f});
			defaultCube.AddComponent<MeshComponent>(cubeData, standardProgram);

			Entity pointlight1 = scene.CreateEntity("Point Light 1");
			pointlight1.AddComponent<TransformComponent>(glm::vec3{2.f, 3.f, 2.f});
			pointlight1.AddComponent<PointLightComponent>(glm::vec3{1.f, 0.f, 0.f});

			Entity pointlight2 = scene.CreateEntity("Point Light 2");
			pointlight2.AddComponent<TransformComponent>(glm::vec3{-3.f, 3.f, -3.f});
			pointlight2.AddComponent<PointLightComponent>(glm::vec3{0.2f, 0.3f, 0.8f});

			Entity pointlight3 = scene.CreateEntity("Point Light 3");
			pointlight3.AddComponent<TransformComponent>(glm::vec3{-3.f, 0.3f, 3.f});
			pointlight3.AddComponent<PointLightComponent>(glm::vec3{0.212f, 0.949f, 0.129f});

			Entity environ = scene.CreateEntity("Environment");
			environ.AddComponent<TransformComponent>(glm::vec3{-6.f, 6.f, -7.f});
			environ.AddComponent<EnvironmentComponent>();

			Entity sunlight = scene.CreateEntity("Sun Light");
			sunlight.AddComponent<TransformComponent>(glm::vec3{-7.f, 5.f, -4.f});
			sunlight.AddComponent<DirectionalLightComponent>(glm::vec3{0.91, 0.882, 0.714});

			Entity glasspane = scene.CreateEntity("Glass Plane");
			glasspane.AddComponent<TransformComponent>(glm::vec3{-1.f, 1.f, 0.f});
			glasspane.AddComponent<MeshComponent>(quadData, standardProgram);
		}
	}

	glm::mat4 BillboardModelMatrix(glm::mat4 model, ViewportCamera& camera) {
		// get camera properties using helper functions
		glm::vec3 cameraPosition = camera.GetPosition();
		glm::vec3 cameraUp = camera.GetUpVector();
		glm::vec3 cameraForward = camera.GetFrontVector();

		glm::vec3 right = glm::normalize(glm::cross(cameraUp, cameraForward));
		glm::vec3 up = glm::cross(cameraForward, right);

		glm::vec3 modelPosition = model[3];
		// apply new rotation basis to model matrix (overwrite rotation)
		model[0] = glm::vec4(right, 0.0f);
		model[1] = glm::vec4(-up, 0.0f);
		model[2] = glm::vec4(-cameraForward, 0.0f);
		model[3] = glm::vec4(modelPosition, 1.0f);// preserve position
		return model;
	}
	void EditorApplication::Loop() {
		glfwPollEvents();
		GLFWwindow* nativeWindow = _globals.window.GetNativeWindow();
		VulkanEngine& engine = _globals.engine;

		if (glfwGetWindowAttrib(nativeWindow, GLFW_ICONIFIED) == GLFW_TRUE) isMinimized = true;
		else isMinimized = false;
		// throttling on minimize
		if (isMinimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));
			return;
		}
		if (engine.resizeRequested) {
			engine.OnResizeWindow(nativeWindow);
			// recreate descriptor set for the viewport image used by imgui
			engine._viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(engine.default_LinearSampler, engine._viewportImage.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}


		ViewportCamera& camera = _globals.camera;
		camera.Update();

		GPU::CameraUBO sceneData = {
				.projectionMatrix = camera.GetProjectionMatrix(),
				.viewMatrix = camera.GetViewMatrix(),
				.position = camera.GetPosition()
		};
		vmaCopyMemoryToAllocation(engine._allocator, &sceneData, engine._cameraDataBuffer.allocation, 0, sizeof(GPU::CameraUBO));

		auto env = _globals.activeContxt.scene.GetEnvironment();
		GPU::LightingUBO lightingData {
				.ambient {
						.Color = env.ambient.Color,
						.Intensity = env.ambient.Intensity
				}
		};
		lightingData.ClearDynamics();

		for (Entity& entity : _globals.activeContxt.scene.GetAllEntitiesWith<DirectionalLightComponent>()) {
			auto entity_transform = entity.GetComponent<TransformComponent>();
			auto entity_light = entity.GetComponent<DirectionalLightComponent>();

			lightingData.directional.Direction = entity_transform.Rotation * glm::vec3(0, -1, 0);

			lightingData.directional.Color = entity_light.directional.Color;
			lightingData.directional.Intensity = entity_light.directional.Intensity;
		}
		int i = 0;
		for (Entity entity : _globals.activeContxt.scene.GetAllEntitiesWith<PointLightComponent>()) {
			auto entity_transform = entity.GetComponent<TransformComponent>();
			auto entity_light = entity.GetComponent<PointLightComponent>();

			lightingData.points[i].Position = entity_transform.Position;

			lightingData.points[i].Color = entity_light.point.Color;
			lightingData.points[i].Intensity = entity_light.point.Intensity;
			lightingData.points[i].Range = entity_light.point.Range;
			i++;
		}
		int j = 0;
//		for (Entity entity : _globals.activeContxt.scene.GetAllEntitiesWith<SpotLightComponent>()) {
//			auto entity_light = entity.GetComponent<SpotLightComponent>();
//			auto entity_transform = entity.GetComponent<TransformComponent>();
//
//			lightingData.spots[i].Position = entity_transform.Position;
//			lightingData.spots[i].Direction = entity_transform.Rotation * glm::vec3(0, -1, 0);
//
//			lightingData.spots[i].Color = entity_light.spot.Color;
//			lightingData.spots[i].Intensity = entity_light.spot.Intensity;
//			lightingData.spots[i].Size = entity_light.spot.Size;
//			lightingData.spots[i].Blend = entity_light.spot.Blend;
//			j++;
//		}
		vmaCopyMemoryToAllocation(engine._allocator, &lightingData, engine._lightingDataBuffer.allocation, 0, sizeof(GPU::LightingUBO));


		{
			engine.Aquire();

			VkCommandBufferBeginInfo cmdbegininfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			vkBeginCommandBuffer(engine.getCurrentFrameData()._commandBuffer, &cmdbegininfo);

			// our main pass includes a color and depth image
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorMSAAImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._depthStencilMSAAImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._entityMSAAImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._entityImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

			// drawing test of objects
			{
				VkClearColorValue entityClear = { -1, 0, 0, 0 };
				VkRenderingAttachmentInfo geometry_entity_attachment = vkinfo::CreateColorAttachmentInfo(engine._entityMSAAImage.imageView, &entityClear, engine._entityImage.imageView, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
				VkRenderingAttachmentInfo geometry_color_attachment = vkinfo::CreateColorAttachmentInfo(engine._colorMSAAImage.imageView, &engine.clearColorValue, engine._colorResolveImage.imageView);
				VkClearDepthStencilValue depthClear = { 1.f, 0 };
				VkRenderingAttachmentInfo geometry_depth_attachment = vkinfo::CreateDepthStencilAttachmentInfo(engine._depthStencilMSAAImage.imageView, &depthClear);

				VkExtent2D renderExtent = engine._viewportImage.GetExtent2D();

				VkRenderingAttachmentInfo color_attachments[] = { geometry_color_attachment, geometry_entity_attachment};
				VkRenderingInfo geometry_render_info = vkinfo::CreateRenderingInfo(renderExtent, std::span(color_attachments), &geometry_depth_attachment);
				vkCmdBeginRenderingKHR(engine.getCurrentFrameData()._commandBuffer, &geometry_render_info);
				vkutil::SetViewport(engine.getCurrentFrameData()._commandBuffer, renderExtent);
				vkutil::SetScissor(engine.getCurrentFrameData()._commandBuffer, renderExtent);



				DescriptorWriter writer {};
				writer.WriteBuffer(0, engine._cameraDataBuffer.buffer, sizeof(GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				writer.WriteBuffer(1, engine._lightingDataBuffer.buffer, sizeof (GPU::LightingUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				VkDescriptorSet centralDescriptor = engine.getCurrentFrameData()._frameDescriptors.Allocate(engine._vkDevice, engine._centralDescriptorSetLayout);
				writer.UpdateSet(engine._vkDevice, centralDescriptor);

				// first drawing
				{
					vkCmdBindDescriptorSets(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._standardPL, 0, 1, &centralDescriptor, 0, nullptr);
					if (_globals.editorGui._viewportMode == ViewportModes::SHADED || _globals.editorGui._viewportMode == ViewportModes::SOLID_WIREFRAME) {
						// main drawing of all entities!!!
						vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._standardPipeline);
						vkCmdSetPolygonModeEXT(engine.getCurrentFrameData()._commandBuffer, VK_POLYGON_MODE_FILL);
						this->StandardDrawMeshes();
					} else if (_globals.editorGui._viewportMode == ViewportModes::WIREFRAME) {
						vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._plaincolorTrianglePipeline);
						vkCmdSetPolygonModeEXT(engine.getCurrentFrameData()._commandBuffer, VK_POLYGON_MODE_LINE);
						this->StandardDrawMeshes();
					}
					if (_globals.editorGui._viewportMode == ViewportModes::SOLID_WIREFRAME) {
						vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._plaincolorTrianglePipeline);
						vkCmdSetPolygonModeEXT(engine.getCurrentFrameData()._commandBuffer, VK_POLYGON_MODE_LINE);
						this->StandardaBitBiggerDrawMeshes();
					}
				}

				GPU::ExtraUBO extra {
						.color = {0, 1, 0, 1}
				};
				vmaCopyMemoryToAllocation(engine._allocator, &extra, engine._colorBuffer.allocation, 0, sizeof(GPU::ExtraUBO));

				DescriptorWriter writer2 {};
				writer2.WriteBuffer(0, engine._cameraDataBuffer.buffer, sizeof(GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				writer2.WriteImage(1, lightbulb_image.imageView, engine.default_LinearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				writer2.WriteBuffer(2, engine._colorBuffer.buffer, sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				VkDescriptorSet pointimageDescriptor = engine.getCurrentFrameData()._frameDescriptors.Allocate(engine._vkDevice, engine._imageDescriptorSetLayout);
				writer2.UpdateSet(engine._vkDevice, pointimageDescriptor);

				// draw some editor stuff
				if (_globals.editorGui.gridIsEnabled) {
					vkCmdBindDescriptorSets(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._standardPL, 0, 1, &centralDescriptor, 0, nullptr);
					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._plaincolorLinePipeline);
					{ // just grid
						GPU::DrawPushConstants pushConstants = this->gridmesh.constants;
						pushConstants.modelMatrix = glm::translate(glm::mat4(1), {0.f, -0.009f, 0.f});
						vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._standardPL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
						engine.DrawMeshData(this->gridmesh);
					}
					// editor primitive visualizers
					//========================//
					// ----- WIREFRAMES ----- //
					//========================//
					{
						for (Entity entity: _globals.activeContxt.scene.GetAllEntitiesWith<PointLightComponent>()) {
							glm::mat4 model = TransformToModelMatrix(entity.GetComponent<TransformComponent>(), false, false);
							glm::mat4 spheremodel = glm::scale(model, glm::vec3(entity.GetComponent<PointLightComponent>().point.Range));

							GPU::DrawPushConstants pushConstants = {};
							{
								pushConstants.modelMatrix = spheremodel;
								pushConstants.vertexBufferAddress = this->simplespheremesh.constants.vertexBufferAddress;
							}
							vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._standardPL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
							engine.DrawMeshData(this->simplespheremesh);
						}
						for (Entity entity: _globals.activeContxt.scene.GetAllEntitiesWith<DirectionalLightComponent>()) {
							glm::mat4 model = TransformToModelMatrix(entity.GetComponent<TransformComponent>(), false);

							GPU::DrawPushConstants pushConstants = {};
							{
								pushConstants.modelMatrix = model;
								pushConstants.vertexBufferAddress = this->arrowmesh.constants.vertexBufferAddress;
							}
							vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._standardPL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
							engine.DrawMeshData(this->arrowmesh);
						}
					}


					GPU::ExtraUBO extra {
							.color = { 0.f, 1.f, 0.f, 1.f }
					};
					vmaCopyMemoryToAllocation(engine._allocator, &extra, engine._colorBuffer.allocation, 0, sizeof(GPU::ExtraUBO));
					DescriptorWriter writer3 {};
					writer3.WriteBuffer(0, engine._cameraDataBuffer.buffer, sizeof(GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
					writer3.WriteImage(1, lightbulb_image.imageView, engine.default_LinearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					writer3.WriteBuffer(2, engine._colorBuffer.buffer, sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
					pointimageDescriptor = engine.getCurrentFrameData()._frameDescriptors.Allocate(engine._vkDevice, engine._imageDescriptorSetLayout);
					writer3.UpdateSet(engine._vkDevice, pointimageDescriptor);


					vkCmdBindPipeline(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._imagePipeline);
					vkCmdSetPolygonModeEXT(engine.getCurrentFrameData()._commandBuffer, VK_POLYGON_MODE_FILL);
					// editor icon visualizers
					//=================//
					// ---- ICONS ---- //
					//=================//
					{
						vkCmdBindDescriptorSets(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._imagePL, 0, 1, &pointimageDescriptor, 0, nullptr);
						for (Entity entity: _globals.activeContxt.scene.GetAllEntitiesWith<PointLightComponent>()) {
							glm::mat4 model = TransformToModelMatrix(entity.GetComponent<TransformComponent>(), false, false);

							// billboard logic
							model = BillboardModelMatrix(model, camera);
							model = glm::scale(model, glm::vec3{0.5f});

							GPU::DrawPushConstants pushConstantsPlane = {};
							{
								pushConstantsPlane.modelMatrix = model;
								pushConstantsPlane.id = static_cast<uint32_t>(entity.GetHandle());
								pushConstantsPlane.vertexBufferAddress = quadData.constants.vertexBufferAddress;
							}
							vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._imagePL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstantsPlane);
							engine.DrawMeshData(this->quadData);
						}

						GPU::ExtraUBO extra {
								.color = { 1.f, 1.f, 0.f, 1.f }
						};
						vmaCopyMemoryToAllocation(engine._allocator, &extra, engine._colorBuffer.allocation, 0, sizeof(GPU::ExtraUBO));
						DescriptorWriter writer4 {};

						writer4.WriteBuffer(0, engine._cameraDataBuffer.buffer, sizeof (GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
						writer4.WriteImage(1, sun_image.imageView, engine.default_LinearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
						writer4.WriteBuffer(2, engine._colorBuffer.buffer, sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
						pointimageDescriptor = engine.getCurrentFrameData()._frameDescriptors.Allocate(engine._vkDevice, engine._imageDescriptorSetLayout);
						writer4.UpdateSet(engine._vkDevice, pointimageDescriptor);

						vkCmdBindDescriptorSets(engine.getCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine._imagePL, 0, 1, &pointimageDescriptor, 0, nullptr);
						for (Entity entity: _globals.activeContxt.scene.GetAllEntitiesWith<DirectionalLightComponent>()) {
							glm::mat4 model = TransformToModelMatrix(entity.GetComponent<TransformComponent>(), false);

							// billboard logic
							model = BillboardModelMatrix(model, camera);
							model = glm::scale(model, glm::vec3{0.5f});

							GPU::DrawPushConstants pushConstantsPlane = {};
							{
								pushConstantsPlane.modelMatrix = model;
								pushConstantsPlane.id = static_cast<uint32_t>(entity.GetHandle());
								pushConstantsPlane.vertexBufferAddress = this->quadData.constants.vertexBufferAddress;
							}
							vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._imagePL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstantsPlane);
							engine.DrawMeshData(this->quadData);
						}
					}
				}
				vkCmdEndRenderingKHR(engine.getCurrentFrameData()._commandBuffer);

				// copy the resolved image onto the viewport image that is to be shown inside imgui
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorResolveImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				{
					vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._viewportImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
					vkutil::BlitImageToImage(engine.getCurrentFrameData()._commandBuffer, engine._colorResolveImage.image, engine._viewportImage.image, engine._colorResolveImage.GetExtent2D(), engine._viewportImage.GetExtent2D());
					vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._viewportImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorResolveImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

				// render ui
				{
					_globals.editorGui.OnUpdate(); // idk where to put this
					VkRenderingAttachmentInfo ui_color_attachment = vkinfo::CreateColorAttachmentInfo(engine._colorResolveImage.imageView, nullptr);
					VkRenderingInfo ui_rendering_info = vkinfo::CreateRenderingInfo(engine._colorResolveImage.GetExtent2D(), &ui_color_attachment, nullptr);
					vkCmdBeginRenderingKHR(engine.getCurrentFrameData()._commandBuffer, &ui_rendering_info);
					_globals.editorGui.Render();
					vkCmdEndRenderingKHR(engine.getCurrentFrameData()._commandBuffer);
				}
			}
			// present by blitting to swapchain
			vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._colorResolveImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			{
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				vkutil::BlitImageToImage(engine.getCurrentFrameData()._commandBuffer, engine._colorResolveImage.image, engine._vkSwapchainImages[engine._imageIndex], engine._colorResolveImage.GetExtent2D(), engine._vkSwapchianExtent);
				vkutil::TransitionImageLayout(engine.getCurrentFrameData()._commandBuffer, engine._vkSwapchainImages[engine._imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			}
			VK_CHECK(vkEndCommandBuffer(engine.getCurrentFrameData()._commandBuffer));

			engine.Present();

		}
		if (glfwWindowShouldClose(nativeWindow)) { continue_Loop = false; }
	}

	void EditorApplication::StandardDrawMeshes() {
		auto& engine = this->_globals.engine;
		for (Entity entity: _globals.activeContxt.scene.GetAllEntitiesWith<MeshComponent>()) {
			glm::mat4 model = TransformToModelMatrix(entity.GetComponent<TransformComponent>());

			auto entity_mesh = entity.GetComponent<MeshComponent>();

			GPU::DrawPushConstants pushConstants = {};
			{
				pushConstants.modelMatrix = model;
				pushConstants.id = static_cast<uint32_t>(entity.GetHandle());
				pushConstants.vertexBufferAddress = entity_mesh.meshData.constants.vertexBufferAddress;
			}
			vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._standardPL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
			engine.DrawMeshData(entity_mesh.meshData);
		}
	}
	void EditorApplication::StandardaBitBiggerDrawMeshes() {
		auto& engine = this->_globals.engine;
		for (Entity entity : _globals.activeContxt.scene.GetAllEntitiesWith<MeshComponent>()) {
			glm::mat4 model = TransformToModelMatrix(entity.GetComponent<TransformComponent>());
			model = glm::scale(model, glm::vec3(1.0005f));

			auto entity_mesh = entity.GetComponent<MeshComponent>();

			GPU::DrawPushConstants pushConstants = {};
			{
				pushConstants.modelMatrix = model;
				pushConstants.vertexBufferAddress = entity_mesh.meshData.constants.vertexBufferAddress;
			}
			vkCmdPushConstants(engine.getCurrentFrameData()._commandBuffer, engine._standardPL, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
			engine.DrawMeshData(entity_mesh.meshData);
		}
	}

	void EditorApplication::Shutdown() {
		vkDeviceWaitIdle(_globals.engine._vkDevice); // required
		{
			_globals.editorGui.OnDetach();
		}
	}
}