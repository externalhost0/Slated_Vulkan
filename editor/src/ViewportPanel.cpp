//
// Created by Hayden Rivas on 1/16/25.
//
#include <IconFontCppHeaders/IconsLucide.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <Slate/Input.h>
#include <Slate/Components.h>
#include <Slate/Primitives.h>
#include <Slate/MeshGenerators.h>
#include <Slate/VK/vkutil.h>

#include "EditorGui.h"

namespace Slate {

	bool EditorGui::IsMouseInViewportBounds() {
		auto [mx, my] = ImGui::GetMousePos();
		mx -= _viewportBounds[0].x;
		my -= _viewportBounds[0].y;
		glm::vec2 viewportSize = _viewportBounds[1] - _viewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = static_cast<int>(mx);
		int mouseY = static_cast<int>(my);

		if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(viewportSize.x) && mouseY < static_cast<int>(viewportSize.y)) {
			return true;
		} else {
			return false;
		}
	}

	void Menubar() {

	}
	void EditorGui::OnViewportAttach() {
		// scene image
		{
			engine->_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(engine->default_LinearSampler, engine->_viewportImage.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		// editor meshes
		{
			 gridmesh = engine->CreateMesh(GenerateGridVertices(100.f));
		}
		{
			stagbuf = engine->CreateBuffer(sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
		}
	}

	void EditorGui::OnViewportPanelUpdate() {

		if (IsMouseInViewportBounds()) {
			float mouseX = ImGui::GetMousePos().x;
			float mouseY = ImGui::GetMousePos().y;

			float xrel = (mouseX - _viewportBounds[0].x) / _viewportSize.x;
			float yrel = (mouseY - _viewportBounds[0].y) / _viewportSize.y;

			int32_t offsetX = engine->_vkSwapchianExtent.width * xrel;
			int32_t offsetY = engine->_vkSwapchianExtent.height * yrel;

			VkImageSubresourceLayers layer = {};
			layer.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layer.baseArrayLayer = 0;
			layer.layerCount = 1;
			layer.mipLevel = 0;

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			int pack = 1;
			region.bufferImageHeight = pack;
			region.bufferRowLength = pack;
			region.imageOffset = { offsetX, offsetY, 0 };
			region.imageExtent = { 1, 1, 1 };
			region.imageSubresource = layer;

			engine->Immediate_Submit([&](VkCommandBuffer cmd) {
				vkCmdCopyImageToBuffer(cmd, engine->_entityImage.image, VK_IMAGE_LAYOUT_GENERAL, stagbuf.buffer, 1, &region);
			});

			uint32_t data = *static_cast<uint32_t*>(stagbuf.info.pMappedData);
			if (data < sizeof (data)) { // basically not negative one
				hoveredEntity = Entity(static_cast<entt::entity>(data), &pActiveContext->scene);
			} else {
				hoveredEntity = std::nullopt;
			}
		} else {
			hoveredEntity = std::nullopt;
		}

		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
		// reuse logic from first Slate engine
		if (InputSystem::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
			// only enter camera control mode if not already active and viewport window is hovered
			if (!_isCameraControlActive && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
				&& !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()) {
				ImGui::SetWindowFocus();
				pCamera->isFirstMouse = true;
				_isCameraControlActive = true;
				InputSystem::SetInputMode(GLFW_CURSOR_DISABLED);
			}
			// if already active, continue to update
			if (_isCameraControlActive) {
				glm::ivec2 pos = InputSystem::GetMousePosition();
				{ // kinda important
					pCamera->ProcessMouse(pos.x, pos.y);
					pCamera->ProcessKeys();
				}
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			}
		} else if (_isCameraControlActive) {
			_isCameraControlActive = false;
			InputSystem::SetInputMode(GLFW_CURSOR_NORMAL);
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}


		if (ImGui::BeginMenuBar()) {
			// camera menu
			if (ImGui::BeginMenu(ICON_LC_VIDEO" Camera")) {
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
				ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
				// DISPLAY MODES
//				if (ImGui::BeginMenu("Display Modes")) {
//					if (ImGui::Selectable("Shaded", m_ViewportMode == ViewportModes::SHADED))
//						m_ViewportMode = ViewportModes::SHADED;
//					if (ImGui::Selectable("Unshaded NW", m_ViewportMode == ViewportModes::UNSHADED))
//						m_ViewportMode = ViewportModes::UNSHADED;
//					if (ImGui::Selectable("Depth", m_ViewportMode == ViewportModes::DEPTH))
//						m_ViewportMode = ViewportModes::DEPTH;
//					if (ImGui::Selectable("Normals", m_ViewportMode == ViewportModes::NORMALS))
//						m_ViewportMode = ViewportModes::NORMALS;
//					if (ImGui::Selectable("Wireframe", m_ViewportMode == ViewportModes::WIREFRAME))
//						m_ViewportMode = ViewportModes::WIREFRAME;
//					if (ImGui::Selectable("Wireframe Color", m_ViewportMode == ViewportModes::SHADED_WIREFRAME))
//						m_ViewportMode = ViewportModes::SHADED_WIREFRAME;
//					ImGui::EndMenu();
//				}
				ImGui::PopItemFlag();

				ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 8.0f);
				ImGui::PushItemWidth(120.f);

				ImGui::SliderFloat("Camera Speed", &pCamera->cameraSpeed, 1.0f, 10.0f, "%.2f");
				ImGui::SliderFloat("Camera FOV", &pCamera->_fov, 30.0f, 120.0f);
				ImGui::SliderFloat("Camera Near Plane", &pCamera->_zNear, 0.01f, 10.0f);
				ImGui::SliderFloat("Camera Far Plane", &pCamera->_zFar, 1.0f, 1000.0f);
				ImGui::PopStyleVar();
				ImGui::PopItemWidth();

				ImGui::PopStyleVar();
				ImGui::EndMenu();
			}

			// environment menu
			if (ImGui::BeginMenu(ICON_LC_APERTURE" Environment")) {
				// environment color button
				ImGui::Text("Clear Color");
				ImGui::SameLine();

				if (ImGui::ColorButton("Clear Color", ImVec4(engine->clearColorValue.float32[0], engine->clearColorValue.float32[1], engine->clearColorValue.float32[2], engine->clearColorValue.float32[3]),
									   ImGuiColorEditFlags_NoSidePreview,
									   ImVec2(ImGui::GetFontSize() * 3, ImGui::GetTextLineHeight())))
				{
					ImGui::OpenPopup("ev-picker");
				}
				if (ImGui::BeginPopup("ev-picker")) {
					if (ImGui::IsKeyPressed(ImGuiKey_Enter)) ImGui::CloseCurrentPopup();
					ImGui::ColorPicker4("##pickerBg", reinterpret_cast<float*>(&engine->clearColorValue));
					ImGui::EndPopup();
				}
				//toggle viewport helpers, like grid
				if (ImGui::Button("Grid")) {
					gridIsEnabled = !gridIsEnabled;
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();

			// transformations options
			if (ImGui::MenuItem(ICON_LC_MOVE, nullptr, _guizmoOperation == ImGuizmo::OPERATION::TRANSLATE)) {
				_guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			if (ImGui::MenuItem(ICON_LC_ROTATE_CW, nullptr, _guizmoOperation == ImGuizmo::OPERATION::ROTATE)) {
				_guizmoOperation = ImGuizmo::OPERATION::ROTATE;
			}
			if (ImGui::MenuItem(ICON_LC_EXPAND, nullptr, _guizmoOperation == ImGuizmo::OPERATION::SCALE)) {
				_guizmoOperation = ImGuizmo::OPERATION::SCALE;
			}
			ImGui::Separator();
			if (_guizmoSpace == ImGuizmo::MODE::WORLD) {
				if (ImGui::MenuItem(ICON_LC_GLOBE " Global Space"))
					_guizmoSpace = ImGuizmo::MODE::LOCAL;
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal))
					ImGui::SetTooltip("Switch to Local Space");
			} else {
				if (ImGui::MenuItem(ICON_LC_FOCUS " Local Space"))
					_guizmoSpace = ImGuizmo::MODE::WORLD;
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal))
					ImGui::SetTooltip("Switch to Global Space");
			}
			ImGui::EndMenuBar();
		}


		// set viewport bounds to use in other steps
		{
			ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
			ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		}

		ImVec2 size = ImGui::GetContentRegionAvail();
		if (_viewportSize != *((glm::vec2 *) &size) && size.x > 0.0f && size.y > 0.0f) {
			pCamera->OnResize(size.x, size.y);
			_viewportSize = { size.x, size.y };
			fmt::print("Viewport Change\n");
		}
		sceneTexture = reinterpret_cast<ImTextureID>(engine->_viewportImageDescriptorSet); // we need to re assign sceneTexture variable for when the descriptorSet changes via resize
		ImGui::Image(sceneTexture, ImVec2{_viewportSize.x, _viewportSize.y}); // important part

		// gizmo functionality
		{
			if (pActiveContext->entity && !_isCameraControlActive && pActiveContext->entity->HasComponent<TransformComponent>()) {
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				// properly set bounds as these are specific to the viewport image pane not just the window
				ImGuizmo::SetRect(_viewportBounds[0].x, _viewportBounds[0].y,
								  _viewportBounds[1].x - _viewportBounds[0].x,
								  _viewportBounds[1].y - _viewportBounds[0].y);

				// SNAP logic
				bool snap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
				// the snap value to use for all operations but rotate
				float snapValue = 0.5f;
				if (_guizmoOperation == ImGuizmo::OPERATION::ROTATE)
					snapValue = 45.0f;
				// put the snapping scale on each axis
				float snapValues[3] = { snapValue, snapValue, snapValue };

				auto& entityTC = pActiveContext->entity->GetComponent<TransformComponent>();
				// more snapping logic to round position if i start snapping
				glm::mat4 transformMatrix = glm::translate(glm::mat4(1.f), entityTC.Position) *
									  glm::mat4_cast(entityTC.Rotation) *
									  glm::scale(glm::mat4(1.f), entityTC.Scale);

				ImGuizmo::Manipulate(glm::value_ptr(pCamera->GetViewMatrix()),
									 glm::value_ptr(pCamera->GetProjectionMatrix()),
									 _guizmoOperation, _guizmoSpace,
									 glm::value_ptr(transformMatrix), nullptr, snap ? snapValues : nullptr);

				// now apply the transformations
				if (ImGuizmo::IsUsing()) {
					glm::vec3 position;
					glm::quat rotation;
					glm::vec3 scale;

					glm::vec3 skew;
					glm::vec4 perspective;
					glm::decompose(transformMatrix, scale, rotation, position, skew, perspective);

					if (_guizmoOperation == ImGuizmo::OPERATION::TRANSLATE) {
						entityTC.Position = position;
					}
					if (_guizmoOperation == ImGuizmo::OPERATION::ROTATE) {
						entityTC.Rotation = glm::normalize(rotation);
					}
					if (_guizmoOperation == ImGuizmo::OPERATION::SCALE) {
						entityTC.Scale = scale;
					}
				}
			}
			// mouse interaction with clicked object must be after guizmo
			// required so that overlapping guizmo and different object dont interfere
			if (InputSystem::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
				if (hoveredEntity && !ImGuizmo::IsUsing())
					pActiveContext->entity = hoveredEntity;
			}
			// right now will currently go off in menu that overlaps the image
			if (ImGui::IsMouseDoubleClicked(GLFW_MOUSE_BUTTON_LEFT) && IsMouseInViewportBounds() && !hoveredEntity) {
				pActiveContext->entity = std::nullopt;
			}
		}
		ImGui::End();
	}
}