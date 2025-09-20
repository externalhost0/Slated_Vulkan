//
// Created by Hayden Rivas on 6/4/25.
//
#include "Slate/Application.h"
#include "Slate/ECS/Entity.h"
#include "Slate/ECS/Scene.h"
#include "Slate/Filesystem.h"
#include "Slate/MeshGenerators.h"
#include "Slate/Primitives.h"
#include "Slate/SceneTemplates.h"

#include "Slate/VK/vkutil.h"

#include <IOSurface/IOSurface.h>

#include <volk/volk.h>

namespace Slate {
	class App : public Application {
		void onInitialize() override;
		void onTick() override;
		void onRender() override;
		void onShutdown() override;

	public:
		bool isEditorMode = false;
	};

	struct Camera {
		enum class ProjectionType {
			Perspective,
			Orthographic
		};

		glm::vec3 _frontVector = {0, 0, -1};
		glm::vec3 _upVector = {0, 1, 0};

		glm::vec3 _position = {0.f, 0.f, 5.f};

		glm::mat4 _projectionMatrix;
		glm::mat4 _viewMatrix;

		float _near = 0.1f;
		float _far = 100.f;
		float _fov = 65.f;
		float _aspectRatio = 1.33f; // TODO: find better way to default this

		float _unitSize = 10.0f;
		float _left = -_unitSize, _right = _unitSize, _bottom = -_unitSize, _top = _unitSize;

		ProjectionType projectionType = ProjectionType::Perspective;
	};

	struct ViewportCamera : public Camera {
		void ProcessKeys(GLFWwindow *window, double deltaTime);
		void ProcessMouse(int xpos, int ypos);
		void updateAspect(float w, float h) {
			if (w/h <= 0) return;
			this->_aspectRatio = w / h;
			if (projectionType == ProjectionType::Orthographic) {
				this->setOrthoHeight(_unitSize);
			}
		}
	private:
		void setOrthoHeight(float h) {
			_unitSize = h;

			float halfHeight = h / 2.0f;
			float halfWidth = halfHeight * _aspectRatio;
			_left = -halfWidth;
			_right = halfWidth;
			_bottom = -halfHeight;
			_top = halfHeight;
		}
	private:
		const float MOUSE_SENSITIVITY = 0.1f;
	public:
		float cameraSpeed = 6.f;
		bool isFirstMouse = false;
	private: // for mouse operations
		float yaw{};
		float pitch{};
		int lastx{}, lasty{};
	};
	ViewportCamera _camera;

	bool ispressed(GLFWwindow* window, int key) {
		return glfwGetKey(window, key) == GLFW_PRESS;
	}
	void ViewportCamera::ProcessKeys(GLFWwindow* window, double deltaTime) {
		float realSpeed;

		if (ispressed(window, GLFW_KEY_LEFT_SHIFT)) realSpeed = cameraSpeed * 1.75f;
		else realSpeed = cameraSpeed;

		float adjustedSpeed = realSpeed * static_cast<float>(deltaTime);

		if (ispressed(window, GLFW_KEY_W))
			_position += adjustedSpeed * _frontVector;
		if (ispressed(window, GLFW_KEY_S))
			_position -= adjustedSpeed * _frontVector;
		if (ispressed(window, GLFW_KEY_A))
			_position -= glm::normalize(glm::cross(_frontVector, _upVector)) * adjustedSpeed;
		if (ispressed(window, GLFW_KEY_D))
			_position += glm::normalize(glm::cross(_frontVector, _upVector)) * adjustedSpeed;
		if (ispressed(window, GLFW_KEY_SPACE))
			_position += adjustedSpeed * _upVector;
		if (ispressed(window, GLFW_KEY_LEFT_CONTROL))
			_position -= adjustedSpeed * _upVector;

	}
	void ViewportCamera::ProcessMouse(int xpos, int ypos) {
		// first part: resolve mouse movement
		if (isFirstMouse) {
			lastx = xpos;
			lasty = ypos;
			isFirstMouse = false;
		}

		int xoffset = xpos - lastx;
		int yoffset = lasty - ypos;// reversed since y-coordinates go from bottom to top

		lastx = xpos;
		lasty = ypos;
		// second part: calculate new front facing vector
		yaw += static_cast<float>(xoffset) * MOUSE_SENSITIVITY;
		pitch += static_cast<float>(yoffset) * MOUSE_SENSITIVITY;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.z = -cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
		direction.y = sinf(glm::radians(pitch));
		direction.x = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
		_frontVector = glm::normalize(direction);
	}

	void updateMatrices(ViewportCamera& camera) {
		camera._viewMatrix = glm::lookAt(camera._position, camera._position + camera._frontVector, camera._upVector);
		switch (camera.projectionType) {
			case Camera::ProjectionType::Perspective:
				camera._projectionMatrix = glm::perspective(glm::radians(camera._fov), camera._aspectRatio, camera._near, camera._far);
				break;
			case Camera::ProjectionType::Orthographic:
				camera._projectionMatrix = glm::ortho(camera._left, camera._right, camera._bottom, camera._top, camera._near, camera._far);
				break;
		}
	}




	glm::mat4 TransformToModelMatrix(const TransformComponent& component, bool isScalable = true, bool isRotatable = true) {
		auto model = glm::mat4(1);
		model = glm::translate(model, component.global.position);
		// some models we dont want scale to affect them, this is mostly for editor visualizers
		if (isRotatable) model = model * glm::mat4_cast(component.global.rotation);
		if (isScalable) model = glm::scale(model, component.global.scale);
		return model;
	}

	InternalPipelineHandle shadedModePipeline;

	InternalTextureHandle colorResolveImage;
	InternalTextureHandle colorMSAAImage;
	InternalTextureHandle depthStencilMSAAImage;
	InternalTextureHandle entityResolveImage;
	InternalTextureHandle entityMSAAImage;
	InternalTextureHandle outlineImage;

	ShaderResource standardShader;
	ShaderResource primitiveShader;

	std::unordered_map<MeshPrimitiveType, MeshData> defaultPrimitives;
	ViewportCamera viewportCamera;

	ResourcePool<MeshResource> _meshPool;
	ResourcePool<ScriptResource> _scriptPool;
	ResourcePool<ShaderResource> _shaderPool;
	ResourcePool<TextureResource> _texturePool;

	struct Context
	{
		Optional<GameEntity> activeEntity = std::nullopt;
		Optional<GameEntity> hoveredEntity = std::nullopt;
		Scene* scene = nullptr;
	} ctx;

	void buildPrimitveMap(GX& _gx) {
		MeshData quadMeshData = _gx.createMesh(Primitives::quadVertices, Primitives::quadIndices);
		MeshData planeMeshData = _gx.createMesh(Primitives::planeVertices, Primitives::quadIndices);
		MeshData cubeMeshData = _gx.createMesh(Primitives::cubeVertices, Primitives::cubeIndices);
		std::vector<Vertex> vertices = {};
		std::vector<uint32_t> indices = {};
		GenerateSphere(vertices, indices, 1.0f, 15, 15);
		MeshData sphereMeshData = _gx.createMesh(vertices, indices);

		// fill in defaults
		defaultPrimitives[MeshPrimitiveType::Quad] = quadMeshData;
		defaultPrimitives[MeshPrimitiveType::Plane] = planeMeshData;
		defaultPrimitives[MeshPrimitiveType::Cube] = cubeMeshData;
		defaultPrimitives[MeshPrimitiveType::Sphere] = sphereMeshData;
	}



	void App::onInitialize() {
		if (!isEditorMode) {
			// take user start toml
			WindowSpec spec = {};
			spec.resizeable = true;
			spec.videomode = VideoMode::Windowed;
			spec.title = "Slate App Window";
			createWindow(spec);
		}
		VulkanInstanceInfo vk_info = {
				.app_name = "Slate Example App",
				.app_version = {0, 0, 1},
				.engine_name = "Slate Engine",
				.engine_version = {0, 0, 1}
		};
		_gx.create(vk_info, _window.getGLFWWindow());
		Filesystem::SetRelativePath("/Users/hayde/Projects/Personal/Slated/Source/EditorOLD/");
		ctx.scene = new Scene;

		buildPrimitveMap(_gx);

		VkExtent2D extent2D;
		if (isEditorMode) {
			extent2D = { 1280, 720 };
		} else {
			extent2D = _gx.getSwapchainExtent();
		}

		colorResolveImage = _gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Sampled | TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.debugName = "Color Resolve Image",
				.exportMemory = true,
		});
		colorMSAAImage = _gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X4,
				.usage = TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Memoryless,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.debugName = "Color MSAA Image"
		});
		depthStencilMSAAImage = _gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X4,
				.usage = TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Memoryless,
				.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.debugName = "DepthStencil Image"
		});
		entityResolveImage = _gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Attachment | TextureUsageBits::TextureUsageBits_Storage,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R32_UINT,
				.debugName = "Entity Image"
		});
		entityMSAAImage = _gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X4,
				.usage = TextureUsageBits::TextureUsageBits_Attachment,
				.storage = StorageType::Memoryless,
				.format = VK_FORMAT_R32_UINT,
				.debugName = "Entity MSAA Image"
		});
		outlineImage = _gx.createTexture({
				.dimension = extent2D,
				.samples = SampleCount::X1,
				.usage = TextureUsageBits::TextureUsageBits_Attachment | TextureUsageBits::TextureUsageBits_Sampled,
				.storage = StorageType::Device,
				.format = VK_FORMAT_R8_UNORM,
				.debugName = "Outline Selection Image"
		});

		standardShader.loadResource(Filesystem::GetRelativePath("shaders/standard.slang"));
		standardShader.assignHandle(_gx.createShader({
				.spirvBlob = standardShader.requestCode()
		}));
		primitiveShader.loadResource(Filesystem::GetRelativePath("shaders/EditorEXT/editor_primitives.slang"));
		primitiveShader.assignHandle(_gx.createShader({
				.spirvBlob = primitiveShader.requestCode(),
				.pushConstantSize = primitiveShader.getPushSize()
		}));


		const PipelineSpec::AttachmentFormats standardFormats = {
				.colorFormats = {
						_gx.getTextureFormat(colorMSAAImage),
						_gx.getTextureFormat(entityMSAAImage)
				},
				.depthFormat = _gx.getTextureFormat(depthStencilMSAAImage)
		};
		const PipelineSpec::AttachmentFormats noidFormats = {
				.colorFormats = {
						_gx.getTextureFormat(colorMSAAImage),
				},
				.depthFormat = _gx.getTextureFormat(depthStencilMSAAImage)
		};
		const PipelineSpec::AttachmentFormats maskFormats = {
				.colorFormats = {
						_gx.getTextureFormat(outlineImage)
				}
		};
		shadedModePipeline = _gx.createPipeline({
				.topology = TopologyMode::TRIANGLE,
				.polygon = PolygonMode::FILL,
				.blend = BlendingMode::OFF,
				.cull = CullMode::BACK,
				.multisample = SampleCount::X4,
				.formats = standardFormats,
				.shaderhandle = standardShader.getHandle()
		});

//		glfwSetInputMode(_window.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		GameEntity sphere = ctx.scene->createEntity("Cube");
		sphere.addComponent<TransformComponent>().global.position = glm::vec3{-2.f, 4.f, 0.f};
		sphere.addComponent<GeometryPrimitiveComponent>().mesh_type = MeshPrimitiveType::Cube;

		GameEntity quady = ctx.scene->createEntity("Quad");
		quady.addComponent<TransformComponent>().global.position = glm::vec3{1.f, 1.f, 1.f};
		quady.addComponent<GeometryPrimitiveComponent>().mesh_type = MeshPrimitiveType::Quad;

		GameEntity plight1 = ctx.scene->createEntity("Point Light 1");
		plight1.addComponent<PointLightComponent>().point.Color = {1, 0, 0};
	}
	void App::onTick() {
		CommandBuffer& cmd = _gx.acquireCommand();

		_gx._clearColor = {0.7, 0.7, 0.7, 1};

		RenderPass first = {
				.color = {
						RenderPass::ColorAttachmentDesc{
								.texture = colorMSAAImage,
								.resolveTexture = colorResolveImage,
								.loadOp = LoadOperation::CLEAR,
								.storeOp = StoreOperation::STORE,
								.clear = _gx._clearColor
						},
						RenderPass::ColorAttachmentDesc{
								.texture = entityMSAAImage,
								.resolveTexture = entityResolveImage,
								.resolveMode = ResolveMode::SAMPLE_ZERO,
								.loadOp = LoadOperation::CLEAR,
								.storeOp = StoreOperation::STORE,
								.clear = RGBA{-1, 0, 0, 0}
						}
				},
				.depth = {
						.texture = depthStencilMSAAImage,
						.loadOp = LoadOperation::CLEAR,
						.storeOp = StoreOperation::NO_CARE,
						.clear = 1.f
				}
		};



		// you must update buffer before using it in push constants
		{
//			double x, y;
//			glfwGetCursorPos(_window.getGLFWWindow(), &x, &y);
//			_camera.updateAspect(_window.getWidth(), _window.getHeight());
//			_camera.ProcessMouse(x, y);
//			_camera.ProcessKeys(_window.getGLFWWindow(), _apptime.getDeltaTime());
			updateMatrices(_camera);
			GPU::CameraData cameraData = {
					.projectionMatrix = _camera._projectionMatrix,
					.viewMatrix = _camera._viewMatrix,
					.position = _camera._position
			};
			AmbientLightComponent env = ctx.scene->GetEnvironment();
			GPU::LightingData lightingData {
					.ambient{
							.Color = env.ambient.Color,
							.Intensity = env.ambient.Intensity
					}
			};
			lightingData.ClearDynamics();

			for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<DirectionalLightComponent>()) {
				const TransformComponent entity_transform = entity.getComponent<TransformComponent>();
				const DirectionalLightComponent entity_light = entity.getComponent<DirectionalLightComponent>();

				lightingData.directional.Direction = entity_transform.global.rotation * glm::vec3(0, -1, 0);
				// stuff from properties panel
				lightingData.directional.Color = entity_light.directional.Color;
				lightingData.directional.Intensity = entity_light.directional.Intensity;
			}
			int i = 0;
			for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<PointLightComponent>()) {
				const TransformComponent entity_transform = entity.getComponent<TransformComponent>();
				const PointLightComponent entity_light = entity.getComponent<PointLightComponent>();

				lightingData.points[i].Position = entity_transform.global.position;
				// stuff from properties panel
				lightingData.points[i].Color = entity_light.point.Color;
				lightingData.points[i].Intensity = entity_light.point.Intensity;
				lightingData.points[i].Range = entity_light.point.Range;
				i++;
			}
			i = 0;
			for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<SpotLightComponent>()) {
				const TransformComponent entity_transform = entity.getComponent<TransformComponent>();
				const SpotLightComponent entity_light = entity.getComponent<SpotLightComponent>();

				lightingData.spots[i].Position = entity_transform.global.position;
				lightingData.spots[i].Direction = entity_transform.global.rotation * glm::vec3(0, -1, 0);
				// stuff from properties panel
				lightingData.spots[i].Color = entity_light.spot.Color;
				lightingData.spots[i].Intensity = entity_light.spot.Intensity;
				lightingData.spots[i].Size = entity_light.spot.Size;
				lightingData.spots[i].Blend = entity_light.spot.Blend;
				i++;
			}
			const GPU::PerFrameData perframedata = {
					.camera = cameraData,
					.lighting = lightingData,
					.time = (float) _apptime.getElapsedTime(),
					.resolution = {_gx.getSwapchainExtent().width, _gx.getSwapchainExtent().height}};
			cmd.cmdUpdateBuffer(_gx._globalBufferHandle, perframedata);
		}

//		cmd.cmdTransitionLayout(colorMSAAImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//		cmd.cmdTransitionLayout(depthStencilMSAAImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//		cmd.cmdTransitionLayout(entityResolveImage, VK_IMAGE_LAYOUT_GENERAL);
//		cmd.cmdTransitionLayout(entityMSAAImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		InternalTextureHandle swapTexHandle = _gx.acquireCurrentSwapchainTexture();
		cmd.cmdBeginRendering(first);
		{
			// for shaded
			cmd.cmdBindRenderPipeline(shadedModePipeline);
			cmd.cmdBindDepthState({
					.compareOp = CompareOperation::CompareOp_Less,
					.isDepthWriteEnabled = true,
			});

			for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<GeometryPrimitiveComponent>()) {
				const MeshPrimitiveType type = entity.getComponent<GeometryPrimitiveComponent>().mesh_type;
				if (type == MeshPrimitiveType::Empty) continue;
				const MeshData &mesh = defaultPrimitives[type];

				GPU::PerObjectData constants = {
						.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
						.vertexBufferAddress = _gx.gpuAddress(mesh.getVertexBufferHandle()),
						.id = (uint32_t) entity.getHandle(),
				};
				cmd.cmdPushConstants(constants);
				cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
				cmd.cmdDrawIndexed(mesh.getIndexCount());
			}
//			for (const GameEntity& entity : ctx.scene->GetAllEntitiesWithEXT<GeometryGLTFComponent>()) {
//				const auto meshSource = _meshPool.get(entity.getComponent<GeometryGLTFComponent>().handle);
//				for (int k = 0; k < meshSource->getMeshCount(); k++) {
//					const MeshData &mesh = meshSource->getBuffers()[k];
//					GPU::PushConstants_EditorPrimitives constants = {
//							.modelMatrix = TransformToModelMatrix(entity.getComponent<TransformComponent>()),
//							.vertexBufferAddress = _gx.gpuAddress(mesh.getVertexBufferHandle()),
//							.color = {1, 0, 0}// we just keep red for now
//					};
//					cmd.cmdPushConstants(constants);
//					cmd.cmdBindIndexBuffer(mesh.getIndexBufferHandle());
//					cmd.cmdDrawIndexed(mesh.getIndexCount());
//				}
//			}
		}
		cmd.cmdEndRendering();


		cmd.cmdTransitionLayout(colorResolveImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		cmd.cmdTransitionSwapchainLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		cmd.cmdBlitToSwapchain(colorResolveImage);
		cmd.cmdTransitionSwapchainLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		_gx.submitCommand(cmd, swapTexHandle);
	}
	void App::onRender() {
	}
	void App::onShutdown() {
	}
}

int main(int argc, char* argv[]) {
	Slate::App app;
	app.isEditorMode = false;
	if (argc > 1 && strcmp(argv[1], "--editor") != 0)
		app.isEditorMode = true;
	app.run();
	return 0;
}