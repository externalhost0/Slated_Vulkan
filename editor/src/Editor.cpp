//
// Created by Hayden Rivas on 1/8/25.
//

// external headers

#include <imgui_impl_vulkan.h>
#include <volk.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <span>
#include <iostream>


// Slate Headers
#include <Slate/Resources/ShaderResource.h>
#include <Slate/Resources/MeshResource.h>
#include <Slate/Components.h>
#include <Slate/Debug.h>
#include <Slate/Entity.h>
#include <Slate/Filesystem.h>
#include <Slate/Logger.h>
#include <Slate/MeshGenerators.h>
#include <Slate/PipelineBuilder.h>
#include <Slate/Primitives.h>
#include <Slate/SceneTemplates.h>
#include <Slate/VK/vkinfo.h>
#include <Slate/VK/vkutil.h>
#include <Slate/Window.h>

// editor headers
#include "Editor.h"
#include "Fonts.h"
#include "imgui_internal.h"
// icons!
#include <IconFontCppHeaders/IconsLucide.h>

namespace Slate {

	static void glfw_error_callback(int error, const char *description) {
		fprintf(stderr, "[GLFW] Error %d: %s\n", error, description);
	}
	static void check_vk_result(VkResult err) {
		if (err == 0) return;
		fprintf(stderr, "[Vulkan] Error: VkResult = %d\n", err);
		if (err < 0) abort();
	}

	void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
		auto *pointer = static_cast<Editor *>(glfwGetWindowUserPointer(window));
		pointer->OnKey(key, action, mods);
//		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}
	void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
		auto *pointer = static_cast<Editor *>(glfwGetWindowUserPointer(window));
		pointer->OnMouseButton();
//		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}
	void WindowSizeCallback(GLFWwindow *window, int width, int height) {
		auto *pointer = static_cast<Editor *>(glfwGetWindowUserPointer(window));
		pointer->OnWindowResize(width, height);
	}

	void FontSetup() {
		ImGuiIO &io = ImGui::GetIO();
		// font size controls everything
		float fontSize = 17.f;

// for crisp text on mac retina displays
#if defined(SLATE_OS_MACOS)
		fontSize *= 2.f;
		io.FontGlobalScale = 1.0f / 2.0f;
#else
		io.FontGlobalScale = 1.0F;
#endif
		// main font config, for retina displays
		ImFontConfig fontCfg = {};
		{
			fontCfg.FontDataOwnedByAtlas = false;
			fontCfg.OversampleV = 2;
			fontCfg.OversampleH = 1;
			fontCfg.RasterizerDensity = 1.0f;
			fontCfg.GlyphOffset = ImVec2(-0.4f, 0.0f);
		}
		// icon fonts, how much should we scale the icons
		float iconSize = fontSize * 0.89f;
		ImFontConfig iconFontCfg = {};
		{
			iconFontCfg.MergeMode = true;
			iconFontCfg.GlyphMinAdvanceX = iconSize;                 // Use if you want to make the icon monospaced
			iconFontCfg.GlyphOffset = ImVec2(1.0f, fontSize / 10.0f);// fixes the offset in the text
			iconFontCfg.PixelSnapH = true;
		}

		static const ImWchar ILC_Range[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};

		// the path in which all the fonts are located, figure out a better way to set this later
		const std::string path = Filesystem::GetRelativePath("fonts/");
		// main font, also merged with below icons
		io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize, &iconFontCfg, ILC_Range);

		float mediumSize = 3.0f;
		Fonts::iconMediumFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize + mediumSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize + mediumSize, &iconFontCfg, ILC_Range);

		float largeSize = 10.0f;
		Fonts::iconLargeFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Regular.ttf").c_str(), fontSize + largeSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize + largeSize, &iconFontCfg, ILC_Range);

		// variants of the NotoSans main font
		// must be after the main font as we merge the fonts with the last font that is added to the io
		Fonts::boldFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Bold.ttf").c_str(), fontSize, &fontCfg);
		io.Fonts->AddFontFromFileTTF((path + FONT_ICON_FILE_NAME_LC).c_str(), iconSize, &iconFontCfg, ILC_Range);

		Fonts::largeboldFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Bold.ttf").c_str(), fontSize + 5.0f, &fontCfg);
		Fonts::italicFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Italic.ttf").c_str(), fontSize, &fontCfg);
		Fonts::largeitalicFont = io.Fonts->AddFontFromFileTTF((path + "NotoSans-Italic.ttf").c_str(), fontSize + 5.0f, &fontCfg);
	}

	void StyleStandard(ImGuiStyle *dst = nullptr) {
		ImGui::StyleColorsDark();
		ImGuiStyle *style = dst ? dst : &ImGui::GetStyle();
		{
			style->DockingSeparatorSize = 1.0f;
			style->FrameBorderSize = 1.0f;
			style->FramePadding = ImVec2(10.0f, 4.0f);

			style->TabBarBorderSize = 2.0f;
			style->TabBarOverlineSize = 1.0f;
			style->WindowBorderSize = 1.0f;// for the thick borders on everything
			style->PopupBorderSize = 1.0f; // for those nice borders around things like menus
			style->ScrollbarSize = 13.0f;

			style->WindowRounding = 2.0f;
			style->ScrollbarRounding = 0.0f;
			style->GrabRounding = 1.0f;
			style->FrameRounding = 1.0f;
			style->ChildRounding = 2.0f;
			style->TabRounding = 1.5f;
		}
		ImVec4 *colors = style->Colors;
		{
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.50f, 0.50f, 0.50f, 0.10f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.0f);

			colors[ImGuiCol_Border] = ImVec4(0.39f, 0.39f, 0.35f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.28f);

			colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.84f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);

			colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

			colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

			// lesser colors
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
			colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.74f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);

			// menu bar stuff
			colors[ImGuiCol_Header] = ImVec4(0.03f, 0.03f, 0.03f, 0.72f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.22f, 0.23f, 0.8f);// important
			colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.53f);

			colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.49f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.1f, 0.15f, 0.55f, 1.00f);
			//			colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
			//			colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			//			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);

			//			colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

			// HIGHLIGHT COLORS
			// colors for docking feature on hover
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.10f, 0.10f, 0.10f, 0.70f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.10f, 1.00f, 0.60f, 0.40f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.10f, 1.0f, 0.60f, 1.00f);

			colors[ImGuiCol_CheckMark] = ImVec4(0.10f, 1.0f, 0.60f, 1.00f);
		}
		ImGuizmo::Style *guizmostyle = &ImGuizmo::GetStyle();
		float thickness = 4.0f;
		{
			guizmostyle->TranslationLineThickness = thickness;
			guizmostyle->RotationLineThickness = thickness - 2.0f;
			guizmostyle->ScaleLineThickness = thickness;
			guizmostyle->TranslationLineArrowSize = 7.0f;
			guizmostyle->RotationOuterLineThickness = thickness - 1.0f;
			guizmostyle->HatchedAxisLineThickness = 1.0f;
		}
		ImVec4 *guizmocolors = guizmostyle->Colors;
		{
			guizmocolors[ImGuizmo::DIRECTION_X] = ImVec4(0.858f, 0.243f, 0.113f, 0.929f);
			guizmocolors[ImGuizmo::DIRECTION_Y] = ImVec4(0.603f, 0.952f, 0.282f, 0.929f);
			guizmocolors[ImGuizmo::DIRECTION_Z] = ImVec4(0.227f, 0.478f, 0.972f, 0.929f);
			guizmocolors[ImGuizmo::ROTATION_USING_FILL] = ImVec4(0.727f, 0.478f, 0.072f, 1.0f);
			guizmocolors[ImGuizmo::ROTATION_USING_BORDER] = ImVec4(0.5f, 0.3f, 0.05f, 0.8f);
		}
	}
	void BuildStyle() {
		// necessary to be done early
		FontSetup();
		// our default colors for the user interface
		StyleStandard();
	}

	glm::mat4 TransformToModelMatrix(const TransformComponent &component, bool isScalable = true, bool isRotatable = true) {
		auto model = glm::mat4(1);
		model = glm::translate(model, component.Position);
		// some models we dont want scale to affect them, this is mostly for editor visualizers
		if (isRotatable) model = model * glm::mat4_cast(component.Rotation);
		if (isScalable) model = glm::scale(model, component.Scale);
		return model;
	}
	glm::mat4 BillboardModelMatrix(glm::mat4 model, ViewportCamera &camera) {
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
	void RenderBuffer_LargeEXT(VkCommandBuffer cmd, const Shared<MeshBuffer>& buffer, const GPU::DrawPushConstants &pushConstants, VkPipelineLayout layout) {
		GPU::DrawPushConstants push = pushConstants;
		push.vertexBufferAddress = buffer->GetVBA();
		push.modelMatrix = glm::scale(pushConstants.modelMatrix, glm::vec3(1.0005f));

		vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &push);
		vkCmdBindIndexBuffer(cmd, buffer->GetIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, buffer->GetIndexCount(), 1, 0, 0, 0);
	}
	// recursive
	void DrawEntityLarge_EXT(VkCommandBuffer cmd, Entity &entity, VkPipelineLayout layout, const glm::mat4 &topMatrix = glm::mat4(1)) {
		if (entity.HasComponent<MeshPrimitiveComponent>()) {
			auto &mesh_component = entity.GetImmutableComponent<MeshPrimitiveComponent>();
			GPU::DrawPushConstants push = {};
			push.modelMatrix = TransformToModelMatrix(entity.GetImmutableComponent<TransformComponent>()) * topMatrix;

			RenderBuffer_LargeEXT(cmd, mesh_component.GetMeshBuffer(), push, layout);
		} else if (entity.HasComponent<MeshGLTFComponent>()) {
			auto &mesh_component = entity.GetImmutableComponent<MeshGLTFComponent>();
			GPU::DrawPushConstants push = {};
			push.modelMatrix = TransformToModelMatrix(entity.GetImmutableComponent<TransformComponent>()) * topMatrix;

			for (const auto &buffer: mesh_component.GetMesh()->GetMeshBuffers()) {
				RenderBuffer_LargeEXT(cmd, buffer, push, layout);
			}
		}

		// now recurse through all children if children are present
		if (!entity.HasChildren()) return;
		for (Entity child: entity.GetChildren()) {
			DrawEntityLarge_EXT(cmd, child, layout, TransformToModelMatrix(entity.GetImmutableComponent<TransformComponent>()));
		}
	}
	void RenderBuffer(VkCommandBuffer cmd, const Shared<MeshBuffer>& buffer, GPU::DrawPushConstants& pushConstants, VkPipelineLayout layout) {
		pushConstants.vertexBufferAddress = buffer->GetVBA();

		vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
		vkCmdBindIndexBuffer(cmd, buffer->GetIndexBuffer().getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, buffer->GetIndexCount(), 1, 0, 0, 0);
	}
	// recursive
	void DrawEntity(VkCommandBuffer cmd, Entity& entity, VkPipelineLayout layout, const glm::mat4& topMatrix = glm::mat4(1)) {
		if (entity.HasComponent<MeshPrimitiveComponent>()) {
			const MeshPrimitiveComponent& mesh_component = entity.GetImmutableComponent<MeshPrimitiveComponent>();
			GPU::DrawPushConstants push = {};
			push.id = static_cast<uint32_t>(entity.GetHandle());
			push.modelMatrix = TransformToModelMatrix(entity.GetImmutableComponent<TransformComponent>()) * topMatrix;

			RenderBuffer(cmd, mesh_component.GetMeshBuffer(), push, layout);
		} else if (entity.HasComponent<MeshGLTFComponent>()) {
			const MeshGLTFComponent& mesh_component = entity.GetImmutableComponent<MeshGLTFComponent>();
			GPU::DrawPushConstants push = {};
			push.id = static_cast<uint32_t>(entity.GetHandle());
			push.modelMatrix = TransformToModelMatrix(entity.GetImmutableComponent<TransformComponent>()) * topMatrix;

			for (const auto &buffer: mesh_component.GetMesh()->GetMeshBuffers()) {
				RenderBuffer(cmd, buffer, push, layout);
			}
		}

		// now recurse through all children if children are present
		if (!entity.HasChildren()) return;
		for (Entity& child: entity.GetChildren()) {
			DrawEntity(cmd, child, layout, TransformToModelMatrix(entity.GetImmutableComponent<TransformComponent>()));
		}
	}
	void DrawSolidAll(RenderEngine &engine, Shared<Scene>& scene, VkPipelineLayout pipelineLayout) {
		for (const Shared<Entity>& entity: scene->GetTopLevelEntities()) {
			DrawEntity(engine.GetCurrentFrameData()._commandBuffer, *entity.get(), pipelineLayout);
		}
	}
	// only draws it as solid wireframe
	void DrawSolidAllSlightlyBigger(RenderEngine &engine, Shared<Scene>& scene, VkPipelineLayout pipelinelayout) {
		for (const Shared<Entity>& entity: scene->GetTopLevelEntities()) {
			DrawEntityLarge_EXT(engine.GetCurrentFrameData()._commandBuffer, *entity.get(), pipelinelayout);
		}
	}


	void Editor::OnKey(int key, int action, int mods) {
		// quit
		if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(this->mainWindow.GetGlfwWindow(), true);

		switch (this->_currenthovered) {
			case HoverWindow::ViewportWindow: {
				if (_isCameraControlActive) {

				} else {
					if (key == GLFW_KEY_W) {
						this->_guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
					}
					if (key == GLFW_KEY_E) {
						this->_guizmoOperation = ImGuizmo::OPERATION::ROTATE;
					}
					if (key == GLFW_KEY_R) {
						this->_guizmoOperation = ImGuizmo::OPERATION::SCALE;
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
	void Editor::OnMouseButton() {
	}
	void Editor::OnMouseMove() {
	}
	void Editor::OnWindowResize(int width, int height) {
	}


	void Editor::CreateEditorImages() {
		const VkExtent2D tempextent = this->engine.GetSwapchainExtent();
		// COLOR (final)
		{
			VkImageUsageFlags color_resolve_image_usages = {};
			color_resolve_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			color_resolve_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			color_resolve_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
			this->_colorResolveImage = this->engine.CreateImage(tempextent, VK_FORMAT_R8G8B8A8_UNORM, color_resolve_image_usages, VK_SAMPLE_COUNT_1_BIT, false);
		}
		// MSAA COLOR (draw)
		{
			VkImageUsageFlags color_msaa_image_usages = {};
			color_msaa_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			color_msaa_image_usages |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			this->_colorMSAAImage = this->engine.CreateImage(tempextent, VK_FORMAT_R8G8B8A8_UNORM, color_msaa_image_usages, VK_SAMPLE_COUNT_4_BIT, false);
		}
		// MSAA DEPTH (draw)
		{
			VkImageUsageFlags depth_image_usages = {};
			depth_image_usages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depth_image_usages |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			this->_depthStencilMSAAImage = this->engine.CreateImage(tempextent, VK_FORMAT_D32_SFLOAT_S8_UINT, depth_image_usages, VK_SAMPLE_COUNT_4_BIT, false);
		}
		// ENTITY (editor)
		{
			VkImageUsageFlags entity_image_usages = {};
			entity_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			entity_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
			entity_image_usages |= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			this->_entityImage = this->engine.CreateImage(tempextent, VK_FORMAT_R32_UINT, entity_image_usages, VK_SAMPLE_COUNT_1_BIT, false);
		}
		// MSAA ENTITY (editor)
		{
			VkImageUsageFlags entity_msaa_image_usages = {};
			entity_msaa_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			entity_msaa_image_usages |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			entity_msaa_image_usages |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
			this->_entityMSAAImage = this->engine.CreateImage(tempextent, VK_FORMAT_R32_UINT, entity_msaa_image_usages, VK_SAMPLE_COUNT_4_BIT, false);
		}
		// VIEWPORT (ui)
		{
			VkImageUsageFlags viewport_image_usages = {};
			viewport_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			viewport_image_usages |= VK_IMAGE_USAGE_SAMPLED_BIT;
			this->_viewportImage = this->engine.CreateImage(tempextent, VK_FORMAT_R8G8B8A8_UNORM, viewport_image_usages, VK_SAMPLE_COUNT_1_BIT, false);
		}
	}

	void Editor::Initialize() {
		EXPECT(glfwInit() == GLFW_TRUE, "[GLFW] Init of GLFW failed!");// we need a better place to do this
#if defined(SLATE_DEBUG)
		glfwSetErrorCallback(glfw_error_callback);
#endif

		WindowSpecification spec;
		spec.resizeable = true;
		this->mainWindow = Window(spec);
		this->mainWindow.Initialize();

		// callbacks
		glfwSetWindowUserPointer(this->mainWindow.GetGlfwWindow(), this);
		glfwSetKeyCallback(this->mainWindow.GetGlfwWindow(), KeyCallback);
		glfwSetMouseButtonCallback(this->mainWindow.GetGlfwWindow(), MouseButtonCallback);
		glfwSetWindowSizeCallback(this->mainWindow.GetGlfwWindow(), WindowSizeCallback);

		VulkanInstanceInfo editorInfo {
				.app_name = "Slate Editor",
				.app_version = {0, 0, 1},
				.engine_name = "Slate Engine",
				.engine_version = {0, 0, 1}
		};
		this->engine = RenderEngine(editorInfo);
		this->engine.InjectWindow(this->mainWindow.GetGlfwWindow());
		this->engine.Startup();
		this->engine.CreateDefaultSamplers();
		this->CreateEditorImages();

		// icon loading
		this->lightbulb_image = this->engine.CreateImage(Filesystem::GetRelativePath("textures/icons/lightbulb.png"), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);
		this->sun_image = this->engine.CreateImage(Filesystem::GetRelativePath("textures/icons/sun.png"), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);
		this->spotlight_image = this->engine.CreateImage(Filesystem::GetRelativePath("textures/icons/lamp-ceiling.png"), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);

		this->_cameraDataBuffer = this->engine.CreateBuffer(sizeof(GPU::CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		this->_lightingDataBuffer = this->engine.CreateBuffer(sizeof(GPU::LightingUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		this->_colorBuffer = this->engine.CreateBuffer(sizeof(GPU::ExtraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);



//		DescriptorLayoutBuilder builder;
//		builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // camera
//		builder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // lighting
//		this->_globalDS0Layout = builder.build(this->engine.GetDevice(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
//
		VkPushConstantRange range = {};
		range.offset = 0;
		range.size = sizeof(GPU::DrawPushConstants);
		range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinfo::CreatePipelineLayoutInfo(&range, &_globalDS0Layout);
//		VK_CHECK(vkCreatePipelineLayout(this->engine.GetDevice(), &pipeline_layout_info, nullptr, &this->_globalPipeLayout));

//		const char *dir = std::getenv("HOME");
//		const std::string path = std::string(dir) + "/Downloads/glTF-Sample-Models/2.0/" + "BarramundiFish/glTF/BarramundiFish.gltf";
//		MeshResource fishmesh;
//		fishmesh.LoadResource(path);

		// obviously make sure we created the images before
		std::vector<VkFormat> colorFormats = { this->_colorMSAAImage.getFormat(), this->_entityMSAAImage.getFormat() };
		VkFormat depthFormat = this->_depthStencilMSAAImage.getFormat();
		// creating shader program should:
		// generate all required buffers
		// generate all bindings for pipeline
		// generate all push constants
		// generate itself a pipeline
		ShaderResource standardShaderRes;
		standardShaderRes.LoadResource(Filesystem::GetRelativePath("shaders/standard.slang"));
		standardShaderRes.CompileToSpirv();
		standardShaderRes.CreateVkModule(this->engine.GetDevice());
		PassProperties standardproperties = {
				.blendmode = BlendingMode::OFF,
				.depthmode = DepthMode::LESS,
				.topologymode = TopologyMode::TRIANGLE,
				.polygonmode = PolygonMode::FILL,
				.samplemode = MultisampleMode::X4,
				.cullmode = CullMode::BACK,

				.color_formats = colorFormats,
				.depth_format = depthFormat};
		this->_standardPass = this->engine.CreateShaderPass(standardShaderRes, standardproperties);
		standardShaderRes.DestroyVkModule(this->engine.GetDevice());


		// editor shaders should be hard coded, simplier honestly so i can focus on how actual game/user shaders can be optimized

		// editor primitves
		// both editors use this as set 0
		DescriptorLayoutBuilder layoutbuilder = {};
		layoutbuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // camera
		this->_flatDS0Layout = layoutbuilder.build(this->engine.GetDevice(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		ShaderResource flatcolorShader;
		flatcolorShader.LoadResource(Filesystem::GetRelativePath("shaders/EditorEXT/editor_primitives.slang"));
		flatcolorShader.CompileToSpirv();
		flatcolorShader.CreateVkModule(this->engine.GetDevice());
		PassProperties editorproperties = {
				.blendmode = BlendingMode::ALPHA_BLEND,
				.depthmode = DepthMode::LESS,
				.topologymode = TopologyMode::TRIANGLE,
				.polygonmode = PolygonMode::FILL,
				.samplemode = MultisampleMode::X4,
				.cullmode = CullMode::OFF,

				.color_formats = colorFormats,
				.depth_format = depthFormat
		};
		DescriptorLayoutBuilder layoutbuilder2 = {};
		layoutbuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // extra ubo
		this->_flatcolorDS1Layout = layoutbuilder.build(this->engine.GetDevice(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayout descspan[] = {_flatDS0Layout, _flatcolorDS1Layout};

		PipelineBuilder pipebuilder = {};
		pipebuilder.set_module(flatcolorShader.GetModule());
		pipebuilder.set_blending_mode(editorproperties.blendmode);
		pipebuilder.set_depthtest(editorproperties.depthmode);
		pipebuilder.set_multisampling_mode(editorproperties.samplemode);
		pipebuilder.set_topology_mode(editorproperties.topologymode);
		pipebuilder.set_polygon_mode(editorproperties.polygonmode);
		pipebuilder.set_color_formats(std::span(editorproperties.color_formats));
		pipebuilder.set_depth_format(editorproperties.depth_format);
		VkPipelineLayoutCreateInfo pipeinfo = vkinfo::CreatePipelineLayoutInfo(&range, std::span(descspan));
		VK_CHECK(vkCreatePipelineLayout(this->engine.GetDevice(), &pipeinfo, nullptr, &this->_flatcolorPipeLayout));
		this->_flatcolorTriPipeline = pipebuilder.build(this->engine.GetDevice(), this->_flatcolorPipeLayout);
		// copy most logic from the above pipeline, literally only changing the topology mode to topologymode::list
		pipebuilder.set_module(flatcolorShader.GetModule());
		pipebuilder.set_blending_mode(editorproperties.blendmode);
		pipebuilder.set_depthtest(editorproperties.depthmode);
		pipebuilder.set_multisampling_mode(editorproperties.samplemode);
		pipebuilder.set_topology_mode(TopologyMode::LIST);
		pipebuilder.set_polygon_mode(editorproperties.polygonmode);
		pipebuilder.set_color_formats(std::span(editorproperties.color_formats));
		pipebuilder.set_depth_format(editorproperties.depth_format);
		this->_flatcolorLinePipeline = pipebuilder.build(this->engine.GetDevice(), this->_flatcolorPipeLayout);
		flatcolorShader.DestroyVkModule(this->engine.GetDevice());

		ShaderResource flatimageShader;
		flatimageShader.LoadResource(Filesystem::GetRelativePath("shaders/EditorEXT/editor_images.slang"));
		flatimageShader.CompileToSpirv();
		flatimageShader.CreateVkModule(this->engine.GetDevice());
		layoutbuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); // extra ubo
		layoutbuilder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); // texture
		this->_flatimageDS1Layout = layoutbuilder.build(this->engine.GetDevice(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayout descspan2[] = {_flatDS0Layout, _flatimageDS1Layout};

		pipebuilder.set_module(flatimageShader.GetModule());
		pipebuilder.set_blending_mode(editorproperties.blendmode);
		pipebuilder.set_depthtest(editorproperties.depthmode);
		pipebuilder.set_multisampling_mode(editorproperties.samplemode);
		pipebuilder.set_topology_mode(editorproperties.topologymode);
		pipebuilder.set_polygon_mode(editorproperties.polygonmode);
		pipebuilder.set_color_formats(std::span(editorproperties.color_formats));
		pipebuilder.set_depth_format(editorproperties.depth_format);
		VkPipelineLayoutCreateInfo pipeinfo3 = vkinfo::CreatePipelineLayoutInfo(&range, std::span(descspan2));
		VK_CHECK(vkCreatePipelineLayout(this->engine.GetDevice(), &pipeinfo3, nullptr, &this->_flatimagePipeLayout));
		this->_flatimagePipeline = pipebuilder.build(this->engine.GetDevice(), this->_flatimagePipeLayout);
		flatimageShader.DestroyVkModule(this->engine.GetDevice());


		// gui setup
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO &io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
		io.ConfigDragClickToInputText = true;                // makes single click on sldiers
		io.ConfigWindowsMoveFromTitleBarOnly = true;         // makes it so you can only move windows from the bar, required for viewport functionality when undocked
		io.ConfigInputTextCursorBlink = true;                // enables blinking cursor in text boxes
#if defined(SLATE_OS_MACOS)
		io.ConfigMacOSXBehaviors = true;// changes a ton of stuff, just click on it
#endif

		// init imgui for glfw
		VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
											 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
											 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
											 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
											 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
											 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;
		VkDescriptorPool _imguiDescriptorPool;
		VK_CHECK(vkCreateDescriptorPool(this->engine.GetDevice(), &pool_info, nullptr, &_imguiDescriptorPool));

		ImGui_ImplGlfw_InitForVulkan(this->mainWindow.GetGlfwWindow(), false);

		// vulkan render info from our rendersystem
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = this->engine.GetInstance();
		initInfo.PhysicalDevice = this->engine.GetPhysDevice();
		initInfo.Device = this->engine.GetDevice();
		initInfo.Queue = this->engine.GetGraphicsQueue();
		initInfo.DescriptorPool = _imguiDescriptorPool;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		// dyn rendering requirements
		initInfo.UseDynamicRendering = true;

		VkFormat format = this->_colorResolveImage.getFormat();
		initInfo.PipelineRenderingCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = nullptr,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &format};
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = check_vk_result;
		// init imgui for vulkan
		ImGui_ImplVulkan_Init(&initInfo);

		// personal initialization
		// do our themes setting, imgui fonts and style
		BuildStyle();
		this->engine.Immediate_Submit([&](VkCommandBuffer cmd) {
			ImGui_ImplVulkan_CreateFontsTexture();
		});
		// for viewport
		this->_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(this->engine.default_LinearSampler, this->_viewportImage.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		this->stagbuf = this->engine.CreateBuffer(sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		const std::vector<Vertex> triVertices = {
				{{1.0f,  1.0f, 0.0f}},
				{{-1.0f, 1.0f, 0.0f}},
				{{ 0.0f, -1.0f, 0.0f}}
		};
		this->testmesh = this->engine.CreateMeshBuffer(Primitives::quadVertices, Primitives::quadIndices);

		vertices.clear();
		GenerateGrid(vertices, 100.f);
		this->gridmesh = this->engine.CreateMeshBuffer(vertices);

		vertices.clear();
		GenerateArrow2DMesh(vertices, 2.f, 1.f, 0.5f, 0.8f);
		this->arrowmesh = this->engine.CreateMeshBuffer(vertices);

		vertices.clear();
		GenerateSimpleSphere(vertices, 50);
		this->simplespheremesh = this->engine.CreateMeshBuffer(vertices);

		vertices.clear();
		GenerateSpot(vertices, 10);
		this->spotmesh = this->engine.CreateMeshBuffer(vertices);

		vertices.clear();
		GenerateSphere(vertices, indices, 1.f, 25, 25);
		MeshBuffer sphereData = this->engine.CreateMeshBuffer(vertices, indices);
		// cause we refrence this in loop logic
		this->quadData = this->engine.CreateMeshBuffer(Primitives::quadVertices, Primitives::quadIndices);
		MeshBuffer planeData = this->engine.CreateMeshBuffer(Primitives::planeVertices, Primitives::quadIndices);
		MeshBuffer cubeData = this->engine.CreateMeshBuffer(Primitives::cubeVertices, Primitives::cubeIndices);

		this->scene = CreateShared<Scene>();

		Shared<Entity> cube1 = scene->CreateEntity("Sphere");
		cube1->AddComponent<TransformComponent>(glm::vec3{-2.f, 4.f, 0.f});
		cube1->AddComponent<MeshPrimitiveComponent>(sphereData, MeshPrimitiveType::Sphere);

		Shared<Entity> cube2 = scene->CreateEntity("Quad");
		cube2->AddComponent<TransformComponent>(glm::vec3{1.f, 1.f, 1.f});
		cube2->AddComponent<MeshPrimitiveComponent>(this->quadData, MeshPrimitiveType::Quad);

		Shared<Entity> standardPlane = scene->CreateEntity("Standard Plane");
		standardPlane->AddComponent<MeshPrimitiveComponent>(planeData, MeshPrimitiveType::Plane);
		standardPlane->GetMutableComponent<TransformComponent>().Scale = {10.f, 1.f, 10.f};

		Shared<Entity> defaultCube = scene->CreateEntity("Default Cube");
		defaultCube->AddComponent<TransformComponent>(glm::vec3{-5.f, 1.f, -1.f});
		defaultCube->AddComponent<MeshPrimitiveComponent>(cubeData, MeshPrimitiveType::Cube);

		Shared<Entity> pointlight1 = scene->CreateEntity("Point Light 1");
		pointlight1->AddComponent<TransformComponent>(glm::vec3{2.f, 3.f, 2.f});
		pointlight1->AddComponent<PointLightComponent>(glm::vec3{1.f, 0.f, 0.f});

		Shared<Entity> pointlight2 = scene->CreateEntity("Point Light 2");
		pointlight2->AddComponent<TransformComponent>(glm::vec3{-3.f, 3.f, -3.f});
		pointlight2->AddComponent<PointLightComponent>(glm::vec3{0.2f, 0.3f, 0.8f});

		Shared<Entity> pointlight3 = scene->CreateEntity("Point Light 3");
		pointlight3->AddComponent<TransformComponent>(glm::vec3{-3.f, 0.3f, 3.f});
		pointlight3->AddComponent<PointLightComponent>(glm::vec3{0.212f, 0.949f, 0.129f});

		Shared<Entity> environ = scene->CreateEntity("Environment");
		environ->AddComponent<TransformComponent>(glm::vec3{-6.f, 6.f, -7.f});
		environ->AddComponent<EnvironmentComponent>();

		Shared<Entity> sunlight = scene->CreateEntity("Sun Light");
		sunlight->AddComponent<TransformComponent>(glm::vec3{-7.f, 5.f, -4.f});
		sunlight->AddComponent<DirectionalLightComponent>(glm::vec3{0.91, 0.882, 0.714});

		Shared<Entity> spotlight = scene->CreateEntity("Spot Light 1");
		spotlight->AddComponent<TransformComponent>(glm::vec3{2.f, 4.f, -4.f});
		spotlight->AddComponent<SpotLightComponent>(glm::vec3{0.3f, 0.6f, 0.1f});

		Shared<Entity> glasspane = scene->CreateEntity("Glass Plane");
		glasspane->AddComponent<TransformComponent>(glm::vec3{-1.f, 1.f, 0.f});
		glasspane->AddComponent<MeshPrimitiveComponent>(quadData, MeshPrimitiveType::Quad);

		ImGui_ImplGlfw_InstallCallbacks(this->mainWindow.GetGlfwWindow());
	}



	void Editor::Loop() {
		glfwPollEvents();
		double currentTime = glfwGetTime();

		// throttling on minimize
		if (this->mainWindow.isMinimized()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10000));
			return;
		}
		if (engine.resizeRequested) {
			int w, h;
			glfwGetFramebufferSize(this->mainWindow.GetGlfwWindow(), &w, &h);
			engine.OnResize(w, h);
			// recreate images
			this->engine.DestroyImage(this->_colorMSAAImage);
			this->engine.DestroyImage(this->_depthStencilMSAAImage);
			this->engine.DestroyImage(this->_colorResolveImage);
			this->engine.DestroyImage(this->_entityImage);
			this->engine.DestroyImage(this->_viewportImage);
			this->CreateEditorImages();

			// recreate descriptor set for the viewport image used by imgui

			this->_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(engine.default_LinearSampler, this->_viewportImage.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		camera.UpdateMatrices();
		GPU::CameraUBO sceneData = {
				.projectionMatrix = camera.GetProjectionMatrix(),
				.viewMatrix = camera.GetViewMatrix(),
				.position = camera.GetPosition()};
		this->_cameraDataBuffer.writeToBuffer(engine.GetAllocator(), &sceneData, _cameraDataBuffer.getBufferSize(), 0);

		EnvironmentComponent env = this->scene->GetEnvironment();
		GPU::LightingUBO lightingData{
				.ambient{
						.Color = env.ambient.Color,
						.Intensity = env.ambient.Intensity}};
		lightingData.ClearDynamics();

		for (const auto &entity: scene->GetAllEntitiesWith<DirectionalLightComponent>()) {
			TransformComponent entity_transform = entity->GetImmutableComponent<TransformComponent>();
			DirectionalLightComponent entity_light = entity->GetImmutableComponent<DirectionalLightComponent>();

			lightingData.directional.Direction = entity_transform.Rotation * glm::vec3(0, -1, 0);
			// stuff from properties panel
			lightingData.directional.Color = entity_light.directional.Color;
			lightingData.directional.Intensity = entity_light.directional.Intensity;
		}
		int i = 0;
		for (const auto &entity: scene->GetAllEntitiesWith<PointLightComponent>()) {
			TransformComponent entity_transform = entity->GetImmutableComponent<TransformComponent>();
			PointLightComponent entity_light = entity->GetImmutableComponent<PointLightComponent>();

			lightingData.points[i].Position = entity_transform.Position;
			// stuff from properties panel
			lightingData.points[i].Color = entity_light.point.Color;
			lightingData.points[i].Intensity = entity_light.point.Intensity;
			lightingData.points[i].Range = entity_light.point.Range;
			i++;
		}
		i = 0;
		for (const auto &entity: scene->GetAllEntitiesWith<SpotLightComponent>()) {
			TransformComponent entity_transform = entity->GetImmutableComponent<TransformComponent>();
			SpotLightComponent entity_light = entity->GetImmutableComponent<SpotLightComponent>();

			lightingData.spots[i].Position = entity_transform.Position;
			lightingData.spots[i].Direction = entity_transform.Rotation * glm::vec3(0, -1, 0);
			// stuff from properties panel
			lightingData.spots[i].Color = entity_light.spot.Color;
			lightingData.spots[i].Intensity = entity_light.spot.Intensity;
			lightingData.spots[i].Size = entity_light.spot.Size;
			lightingData.spots[i].Blend = entity_light.spot.Blend;
			i++;
		}
		this->_lightingDataBuffer.writeToBuffer(engine.GetAllocator(), &lightingData, _lightingDataBuffer.getBufferSize(), 0);

		this->Render();

		this->deltaTime = currentTime - this->lastTime;
		this->lastTime = currentTime;
		if (glfwWindowShouldClose(this->mainWindow.GetGlfwWindow())) { this->continueloop = false; }
	}

	void Editor::Render() {
		engine.AquireSwapchainFrame();

		VkCommandBufferBeginInfo cmdbegininfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		vkBeginCommandBuffer(engine.GetCurrentFrameData()._commandBuffer, &cmdbegininfo);

		// our main pass includes a color and depth image
		vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_colorMSAAImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_depthStencilMSAAImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_entityImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_entityMSAAImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// drawing test of objects
		{
			VkClearColorValue entityClear = { -1, 0, 0, 0 };
			VkRenderingAttachmentInfo geometry_entity_attachment = vkinfo::CreateColorAttachmentInfo(this->_entityMSAAImage.getImageView(), &entityClear, this->_entityImage.getImageView(), VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
			VkRenderingAttachmentInfo geometry_color_attachment = vkinfo::CreateColorAttachmentInfo(this->_colorMSAAImage.getImageView(), &engine.clearColorValue, this->_colorResolveImage.getImageView());
			VkClearDepthStencilValue depthClear = { 1.f, 0 };
			VkRenderingAttachmentInfo geometry_depth_attachment = vkinfo::CreateDepthStencilAttachmentInfo(this->_depthStencilMSAAImage.getImageView(), &depthClear);

			VkExtent2D renderExtent = this->_viewportImage.getExtent2D();
			VkRenderingAttachmentInfo color_attachments[] = {geometry_color_attachment, geometry_entity_attachment};
			VkRenderingInfo geometry_render_info = vkinfo::CreateRenderingInfo(renderExtent, std::span(color_attachments), &geometry_depth_attachment);
			vkCmdBeginRenderingKHR(engine.GetCurrentFrameData()._commandBuffer, &geometry_render_info);
			vkutil::SetViewport(engine.GetCurrentFrameData()._commandBuffer, renderExtent);
			vkutil::SetScissor(engine.GetCurrentFrameData()._commandBuffer, renderExtent);

			// not user decided
			DescriptorWriter gwriter{};
			gwriter.WriteBuffer(0, this->_cameraDataBuffer.getBuffer(), sizeof(GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			gwriter.WriteBuffer(1, this->_lightingDataBuffer.getBuffer(), sizeof(GPU::LightingUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			VkDescriptorSet engine_only_descriptor = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_standardPass.set0Layout);
			gwriter.UpdateSet(engine.GetDevice(), engine_only_descriptor);
			vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_standardPass.getPipelineLayout(), 0, 1, &engine_only_descriptor, 0, nullptr);

			// === MAIN === //
			{
				if (this->_viewportMode == ViewportModes::SHADED || this->_viewportMode == ViewportModes::SOLID_WIREFRAME) {
					// main drawing of all entities!!!
					vkCmdBindPipeline(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_standardPass.getPipeline());
					for (const Shared<Entity>& entity: this->scene->GetTopLevelEntities()) {
						DrawEntity(engine.GetCurrentFrameData()._commandBuffer, *entity.get(), this->_standardPass.getPipelineLayout());
					}

//					DescriptorWriter hwriter{};
//					hwriter.WriteBuffer(0, this->_cameraDataBuffer.getBuffer(), sizeof(GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
//					VkDescriptorSet ds2 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatDS0Layout);
//					hwriter.UpdateSet(engine.GetDevice(), ds2);
//
//					vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorPipeLayout, 0, 1, &ds2, 0, nullptr);
//
//					DescriptorWriter h2writer{};
//					h2writer.WriteBuffer(0, this->_colorBuffer.getBuffer(), sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
//					VkDescriptorSet ds3 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatcolorDS1Layout);
//					h2writer.UpdateSet(engine.GetDevice(), ds3);
//
//					vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorPipeLayout, 1, 1, &ds3, 0, nullptr);
//
//
//					vkCmdBindPipeline(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorTriPipeline); // USE LINE ITS BETTER
//					GPU::DrawPushConstants pushConstants = {};
//					pushConstants.modelMatrix = TransformToModelMatrix(scene->GetAllEntities().front()->GetImmutableComponent<TransformComponent>());
//					pushConstants.vertexBufferAddress = this->testmesh.vertexBufferAddress;
//					vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatcolorPipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
//					engine.DrawMeshData_EXT(this->testmesh);
				}


				// ============== //
				// === EDITOR === //
				// ============== //

				// set camera uniform for my editor shaders
				DescriptorWriter writer_0;
				writer_0.WriteBuffer(0, this->_cameraDataBuffer.getBuffer(), sizeof(GPU::CameraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				VkDescriptorSet c_descriptor_0 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatDS0Layout);
				VkDescriptorSet i_descriptor_0 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatDS0Layout);
				writer_0.UpdateSet(engine.GetDevice(), c_descriptor_0);
				writer_0.UpdateSet(engine.GetDevice(), i_descriptor_0);

				vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorPipeLayout, 0, 1, &c_descriptor_0, 0, nullptr);
				vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatimagePipeLayout, 0, 1, &i_descriptor_0, 0, nullptr);

				// set color uniform for flat shader
				GPU::ExtraUBO extra_wireframe{
						.color = {1, 0.675, 0, 1.f}};
				this->_colorBuffer.writeToBuffer(engine.GetAllocator(), &extra_wireframe, _colorBuffer.getBufferSize(), 0);

				DescriptorWriter writer_1;
				writer_1.WriteBuffer(0, this->_colorBuffer.getBuffer(), sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				VkDescriptorSet c_descriptor_1 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatcolorDS1Layout);
				writer_1.UpdateSet(engine.GetDevice(), c_descriptor_1);

				vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorPipeLayout, 1, 1, &c_descriptor_1, 0, nullptr);

				// these are camera mdoes
				if (this->_viewportMode == ViewportModes::WIREFRAME) {
					vkCmdBindPipeline(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorTriPipeline);
					DrawSolidAll(this->engine, this->scene, this->_flatcolorPipeLayout);
				}
				if (this->_viewportMode == ViewportModes::SOLID_WIREFRAME) {
					vkCmdBindPipeline(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorTriPipeline);
					DrawSolidAllSlightlyBigger(this->engine, this->scene, this->_flatcolorPipeLayout);
				}
			}

			GPU::ExtraUBO extra_vis{
					.color = {0.9f, 0.9f, 0.9f, 1.f}};
			this->_colorBuffer.writeToBuffer(engine.GetAllocator(), &extra_vis, _colorBuffer.getBufferSize(), 0);

			DescriptorWriter writer_1;
			writer_1.WriteBuffer(0, this->_colorBuffer.getBuffer(), sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			VkDescriptorSet flatDescriptor = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatcolorDS1Layout);
			writer_1.UpdateSet(engine.GetDevice(), flatDescriptor);

			vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorPipeLayout, 1, 1, &flatDescriptor, 0, nullptr);

			// draw some editor stuff
			if (this->gridEnabled) {
				vkCmdBindPipeline(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatcolorLinePipeline);
				{// just grid
					GPU::DrawPushConstants pushConstants = {};
					pushConstants.modelMatrix = glm::translate(glm::mat4(1), {0.f, -0.009f, 0.f});
					pushConstants.vertexBufferAddress = this->gridmesh.GetVBA();
					vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatcolorPipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
					engine.DrawMeshData_EXT(this->gridmesh);
				}
				// editor primitive visualizers
				//========================//
				// ----- WIREFRAMES ----- //
				//========================//
				{
					for (const auto &entity: scene->GetAllEntitiesWith<PointLightComponent>()) {
						glm::mat4 model = TransformToModelMatrix(entity->GetImmutableComponent<TransformComponent>(), false, false);
						glm::mat4 spheremodel = glm::scale(model, glm::vec3(entity->GetImmutableComponent<PointLightComponent>().point.Range));

						GPU::DrawPushConstants pushConstants = {
								.modelMatrix = spheremodel,
								.vertexBufferAddress = this->simplespheremesh.GetVBA()};
						vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatcolorPipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
						engine.DrawMeshData_EXT(this->simplespheremesh);
					}
					for (const auto &entity: scene->GetAllEntitiesWith<SpotLightComponent>()) {
						glm::mat4 model = TransformToModelMatrix(entity->GetImmutableComponent<TransformComponent>(), false);
						float size = entity->GetImmutableComponent<SpotLightComponent>().spot.Size;
						float halfAngle = glm::radians(size * 0.5f);
						float radius = sin(halfAngle) / sin(glm::radians(5.0f));
						float height = radius / tan(halfAngle);
						glm::mat4 spotmodel = glm::scale(model, glm::vec3{radius, height, radius});

						GPU::DrawPushConstants pushConstants = {
								.modelMatrix = spotmodel,
								.vertexBufferAddress = this->spotmesh.GetVBA()};
						vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatcolorPipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
						engine.DrawMeshData_EXT(this->spotmesh);
					}
					for (const auto &entity: scene->GetAllEntitiesWith<DirectionalLightComponent>()) {
						glm::mat4 model = TransformToModelMatrix(entity->GetImmutableComponent<TransformComponent>(), false);

						GPU::DrawPushConstants pushConstants = {
								.modelMatrix = model,
								.vertexBufferAddress = this->arrowmesh.GetVBA()};
						vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatcolorPipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstants);
						engine.DrawMeshData_EXT(this->arrowmesh);
					}
				}

				GPU::ExtraUBO extra{.color = {0.f, 1.f, 0.f, 1.f}};
				this->_colorBuffer.writeToBuffer(engine.GetAllocator(), &extra, _colorBuffer.getBufferSize(), 0);

				DescriptorWriter writer3{};
				writer3.WriteBuffer(0, this->_colorBuffer.getBuffer(), sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				writer3.WriteImage(1, this->lightbulb_image.getImageView(), engine.default_LinearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				VkDescriptorSet imageDescriptor = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatimageDS1Layout);
				writer3.UpdateSet(engine.GetDevice(), imageDescriptor);

				vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatimagePipeLayout, 1, 1, &imageDescriptor, 0, nullptr);

				vkCmdBindPipeline(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatimagePipeline);
				//=================//
				// ---- ICONS ---- //
				//=================//
				{
					for (const auto &entity: scene->GetAllEntitiesWith<PointLightComponent>()) {
						glm::mat4 model = TransformToModelMatrix(entity->GetImmutableComponent<TransformComponent>(), false, false);

						// billboard logic
						model = BillboardModelMatrix(model, camera);
						model = glm::scale(model, glm::vec3{0.5f});

						GPU::DrawPushConstants pushConstantsPlane = {
								.modelMatrix = model,
								.vertexBufferAddress = this->quadData.GetVBA(),
								.id = static_cast<uint32_t>(entity->GetHandle())};
						vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatimagePipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstantsPlane);
						engine.DrawMeshData_EXT(this->quadData);
					}

					GPU::ExtraUBO extra {.color = {1.f, 1.f, 0.f, 1.f}};
					this->_colorBuffer.writeToBuffer(engine.GetAllocator(), &extra, _colorBuffer.getBufferSize(), 0);

					DescriptorWriter writer4{};
					writer4.WriteBuffer(0, this->_colorBuffer.getBuffer(), sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
					writer4.WriteImage(1, spotlight_image.getImageView(), engine.default_LinearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					VkDescriptorSet imageDescriptor2 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatimageDS1Layout);
					writer4.UpdateSet(engine.GetDevice(), imageDescriptor2);

					vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatimagePipeLayout, 1, 1, &imageDescriptor2, 0, nullptr);

					for (const auto &entity: scene->GetAllEntitiesWith<SpotLightComponent>()) {
						glm::mat4 model = TransformToModelMatrix(entity->GetImmutableComponent<TransformComponent>(), false, false);

						// billboard logic
						model = BillboardModelMatrix(model, camera);
						model = glm::scale(model, glm::vec3{0.5f});

						GPU::DrawPushConstants pushConstantsPlane = {
								.modelMatrix = model,
								.vertexBufferAddress = this->quadData.GetVBA(),
								.id = static_cast<uint32_t>(entity->GetHandle())};
						vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatimagePipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstantsPlane);
						engine.DrawMeshData_EXT(this->quadData);
					}

					extra = {.color = {1.f, 1.f, 0.f, 1.f}};
					this->_colorBuffer.writeToBuffer(engine.GetAllocator(), &extra, _colorBuffer.getBufferSize(), 0);

					DescriptorWriter writer5{};
					writer5.WriteBuffer(0, this->_colorBuffer.getBuffer(), sizeof(GPU::ExtraUBO), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
					writer5.WriteImage(1, sun_image.getImageView(), engine.default_LinearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					VkDescriptorSet imageDescriptor3 = engine.GetCurrentFrameData()._frameDescriptors.Allocate(engine.GetDevice(), this->_flatimageDS1Layout);
					writer5.UpdateSet(engine.GetDevice(), imageDescriptor3);

					vkCmdBindDescriptorSets(engine.GetCurrentFrameData()._commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->_flatimagePipeLayout, 1, 1, &imageDescriptor3, 0, nullptr);

					for (const auto &entity: scene->GetAllEntitiesWith<DirectionalLightComponent>()) {
						glm::mat4 model = TransformToModelMatrix(entity->GetImmutableComponent<TransformComponent>(), false);

						// billboard logic
						model = BillboardModelMatrix(model, camera);
						model = glm::scale(model, glm::vec3{0.5f});

						GPU::DrawPushConstants pushConstantsPlane = {
								.modelMatrix = model,
								.vertexBufferAddress = this->quadData.GetVBA(),
								.id = static_cast<uint32_t>(entity->GetHandle())};
						vkCmdPushConstants(engine.GetCurrentFrameData()._commandBuffer, this->_flatimagePipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &pushConstantsPlane);
						engine.DrawMeshData_EXT(this->quadData);
					}
				}
			}
			vkCmdEndRenderingKHR(engine.GetCurrentFrameData()._commandBuffer);

			// copy the resolved image onto the viewport image that is to be shown inside imgui
			vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_colorResolveImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			{
				vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_viewportImage.getImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				vkutil::BlitImageToImage(engine.GetCurrentFrameData()._commandBuffer, this->_colorResolveImage.getImage(), this->_viewportImage.getImage(), this->_colorResolveImage.getExtent2D(), this->_viewportImage.getExtent2D());
				vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_viewportImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_colorResolveImage.getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			// render ui
			{
				this->GuiUpdate();
				VkRenderingAttachmentInfo ui_color_attachment = vkinfo::CreateColorAttachmentInfo(this->_colorResolveImage.getImageView(), nullptr);
				VkRenderingInfo ui_rendering_info = vkinfo::CreateRenderingInfo(this->_colorResolveImage.getExtent2D(), &ui_color_attachment, nullptr);
				vkCmdBeginRenderingKHR(engine.GetCurrentFrameData()._commandBuffer, &ui_rendering_info);
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), engine.GetCurrentFrameData()._commandBuffer);
				vkCmdEndRenderingKHR(engine.GetCurrentFrameData()._commandBuffer);
			}
		}
		// present by blitting to swapchain
		vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, this->_colorResolveImage.getImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		{
			vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, engine.GetSwapchainImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			vkutil::BlitImageToImage(engine.GetCurrentFrameData()._commandBuffer, this->_colorResolveImage.getImage(), this->engine.GetSwapchainImage(), this->_colorResolveImage.getExtent2D(), this->engine.GetSwapchainExtent());
			vkutil::TransitionImageLayout(engine.GetCurrentFrameData()._commandBuffer, engine.GetSwapchainImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
		VK_CHECK(vkEndCommandBuffer(engine.GetCurrentFrameData()._commandBuffer));

		engine.PresentSwapchainFrame();
	}

	void Editor::Shutdown() {
		vkDeviceWaitIdle(this->engine.GetDevice());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Editor::GuiUpdate() {
		// required imgui opening
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}
		//		ImGui::SetMouseCursor(ImGuiMouseCursor_None);

		// below is all dockspace setup
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton; // flags for our Dockspace, which will be the whole screen

		// set the parent window's position, m_Count, and viewport to match that of the main viewport. This is so the parent window
		// completely covers the main viewport, giving it a "full-screen" feel.
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		// entire window flags
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		// make manipulation inaccessible to the user (no titlebar, resize/move, or navigation)
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// set the parent window's styles to match that of the main viewport:
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);             // No corner rounding on the window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);           // No border around the window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));// remove window padding for "fullscreen" effect

		ImGui::Begin("DockSpace Main", nullptr, window_flags);
		ImGui::PopStyleVar(3);// pop previous styles, (padding, rounding, border size)
		// The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		{
			// everything thats not a dedicated panel
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
					if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
					if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
					if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) {}
					ImGui::Separator();
					if (ImGui::MenuItem("Settings")) {}
					ImGui::Separator();
					if (ImGui::MenuItem("Exit", "esc")) glfwSetWindowShouldClose(this->mainWindow.GetGlfwWindow(), true);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit")) {
					if (ImGui::MenuItem("BLOW UP", "Ctrl+B")) {}
					ImGui::EndMenu();
				}
			}
			ImGui::EndMainMenuBar();

			// everything that is a panel
			{
				OnViewportPanelUpdate();
				OnScenePanelUpdate();
				OnPropertiesPanelUpdate();
				OnAssetPanelUpdate();
			}
			// debug info
			{
				ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
				ImGui::SetNextWindowBgAlpha(0.5f);
				ImGui::Begin("Info - Debug", nullptr, windowFlags);
				ImGui::Text("Frame: %u", engine.GetCurrentFrameNum());
				ImGuiIO& io = ImGui::GetIO();
				ImGui::Text("Platform Name: %s", io.BackendPlatformName ? io.BackendPlatformName : "Unknown Platform");
				ImGui::Text("Backend RenderManager: %s", io.BackendRendererName ? io.BackendRendererName : "Unknown RenderManager");
				ImGui::Text("Capturing Mouse: %s", io.WantCaptureMouse ? "True" : "False");
				ImGui::Text("Display Size: %.1f x %.1f", io.DisplaySize.x, io.DisplaySize.y);
				ImGui::Text("Swapchain Size: %.1u x %.1u", engine.GetSwapchainExtent().width, engine.GetSwapchainExtent().height);
				ImGui::Text("Image Viewport Size: %.1u x %.1u", this->_colorMSAAImage.getExtent2D().width, this->_colorMSAAImage.getExtent2D().height);
				ImGui::Text("Display Framebuffer Scale: %.1f x %.1f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
				ImGui::Text("ImGui Framerate: %.2f", io.Framerate);
				// should be the same if everything is correct
				ImGui::Text("Slate Delta Time: %.2f", this->deltaTime);
				ImGui::Text("ImGui Delta Time: %.2f", io.DeltaTime);
				if (hoveredEntity.has_value())
					ImGui::Text("Hovered Entity: %s | %u", hoveredEntity.value()->GetName().c_str(), static_cast<int>(hoveredEntity.value()->GetHandle()));
				else
					ImGui::Text("Hovered Entity: 'NONE'");
				ImGui::End();
			}
			ImGui::End();// last end statement, dont put more imgui calls after this
		}
		// required imgui ending
		{
			ImGui::Render();
		}
	}
}