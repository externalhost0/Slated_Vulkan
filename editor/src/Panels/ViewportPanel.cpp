//
// Created by Hayden Rivas on 1/16/25.
//

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "ImGuizmo/ImGuizmo.h"

#include "Slate/Components.h"

#include "../Editor.h"
#include "IconFontCppHeaders/IconsLucide.h"


namespace Slate {

	bool Editor::IsMouseInViewportBounds() {
		double mx, my;
		glfwGetCursorPos(this->mainWindow.GetGlfwWindow(), &mx, &my);
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

	void Editor::OnViewportPanelUpdate() {
		// yes all of these checks are required
		// isCameraControlActive: Requires user to not be in fly mode to select
		// ImGui::IsItemHovered(): Requires user to be hovering item, works in tandem with the next check
		// IsMouseInViewportBounds(): Requires the mouse position is actually inside the viewport
		// TODO: move into main input function and make sure color picking isnt on
		if (!_isCameraControlActive && ImGui::IsItemHovered() && IsMouseInViewportBounds()) {
			double mouseX, mouseY;
			glfwGetCursorPos(this->mainWindow.GetGlfwWindow(), &mouseX, &mouseY);

			float xrel = ((float)mouseX - _viewportBounds[0].x) / _viewportSize.x;
			float yrel = ((float)mouseY - _viewportBounds[0].y) / _viewportSize.y;

			auto offsetX = static_cast<int32_t>((float) engine.GetSwapchainExtent().width * xrel);
			auto offsetY = static_cast<int32_t>((float) engine.GetSwapchainExtent().height * yrel);

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

			this->engine.Immediate_Submit([&](VkCommandBuffer cmd) {
				vkCmdCopyImageToBuffer(cmd, this->_entityImage.getImage(), VK_IMAGE_LAYOUT_GENERAL, stagbuf.getBuffer(), 1, &region);
			});

			int32_t data = *static_cast<int32_t*>(stagbuf.getMappedMemory());
			if (data >= 0) { // basically not negative max value of int32t
				hoveredEntity.emplace(this->scene->GetEntityById(static_cast<entt::entity>(data)));
			} else {
				hoveredEntity = std::nullopt;
			}
		} else {
			hoveredEntity = std::nullopt;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // image takes up entire window
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_MenuBar);
		if (ImGui::IsWindowHovered()) this->_currenthovered = HoverWindow::ViewportWindow;

		// reuse logic from first Slate engine
		if (glfwGetMouseButton(this->mainWindow.GetGlfwWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			// only enter camera control mode if not already active and viewport window is hovered
			if (!_isCameraControlActive && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
				&& !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered()) {
				ImGui::SetWindowFocus();
				this->camera.isFirstMouse = true;
				_isCameraControlActive = true;
				glfwSetInputMode(this->mainWindow.GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			// if already active, continue to update
			if (_isCameraControlActive) {
				double posx, posy;
				glfwGetCursorPos(this->mainWindow.GetGlfwWindow(), &posx, &posy);
				{ // kinda important
					this->camera.ProcessMouse((int)posx, (int)posy);
					this->camera.ProcessKeys(this->mainWindow.GetGlfwWindow(), this->deltaTime);
				}
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			}
		} else if (_isCameraControlActive) {
			_isCameraControlActive = false;
			glfwSetInputMode(this->mainWindow.GetGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}

		// set viewport bounds to use in other steps
		{
			ImVec2 viewportPos = ImGui::GetCursorScreenPos(); // top-left corner of the viewport
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();
			_viewportBounds[0] = { viewportPos.x, viewportPos.y };
			_viewportBounds[1] = { viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y };
		}
		ImVec2 size = ImGui::GetContentRegionAvail();
		if (_viewportSize != *((glm::vec2 *) &size) && size.x > 0.0f && size.y > 0.0f) {
			_viewportSize = { size.x, size.y };
			this->camera.UpdateAspect(size.x, size.y);
		}
		sceneTexture = reinterpret_cast<ImTextureID>(this->_viewportImageDescriptorSet); // we need to re assign sceneTexture variable for when the descriptorSet changes via resize
		ImGui::Image(sceneTexture, ImVec2{_viewportSize.x, _viewportSize.y}); // important part
		ImGui::PopStyleVar(); // pop window padding var

		if (ImGui::BeginMenuBar()) {
			// camera menu
			if (ImGui::BeginMenu(ICON_LC_VIDEO" Camera")) {
				ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
				// DISPLAY MODES
				if (ImGui::BeginMenu("Display Modes")) {
					if (ImGui::Selectable("Shaded", _viewportMode == ViewportModes::SHADED))
						_viewportMode = ViewportModes::SHADED;
					if (ImGui::Selectable("Unshaded NW", _viewportMode == ViewportModes::UNSHADED))
						_viewportMode = ViewportModes::UNSHADED;
					if (ImGui::Selectable("Wireframe", _viewportMode == ViewportModes::WIREFRAME))
						_viewportMode = ViewportModes::WIREFRAME;
					if (ImGui::Selectable("Solid Wireframe", _viewportMode == ViewportModes::SOLID_WIREFRAME))
						_viewportMode = ViewportModes::SOLID_WIREFRAME;

					ImGui::EndMenu();
				}
				ImGui::PopItemFlag();

				ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 8.0f);
				ImGui::PushItemWidth(120.f);

				ImGui::SliderFloat("Camera Speed", &this->camera.cameraSpeed, 1.0f, 10.0f, "%.2f");
				ImGui::SliderFloat("Camera FOV", &this->camera.fov, 30.0f, 120.0f);
				ImGui::SliderFloat("Camera Near Plane", &this->camera.near, 0.01f, 10.0f);
				ImGui::SliderFloat("Camera Far Plane", &this->camera.far, 1.0f, 1000.0f);

				ImGui::PopItemWidth();

				ImGui::PopStyleVar();
				ImGui::EndMenu();
			}

			// environment menu
			if (ImGui::BeginMenu(ICON_LC_APERTURE" Environment")) {
				// environment color button
				ImGui::Text("Clear Color");
				ImGui::SameLine();

				if (ImGui::ColorButton("Clear Color", ImVec4(engine.clearColorValue.float32[0], engine.clearColorValue.float32[1], engine.clearColorValue.float32[2], engine.clearColorValue.float32[3]),
									   ImGuiColorEditFlags_NoSidePreview,
									   ImVec2(ImGui::GetFontSize() * 3, ImGui::GetTextLineHeight())))
				{
					ImGui::OpenPopup("ev-picker");
				}
				if (ImGui::BeginPopup("ev-picker")) {
					if (ImGui::IsKeyPressed(ImGuiKey_Enter)) ImGui::CloseCurrentPopup();
					ImGui::ColorPicker4("##pickerBg", reinterpret_cast<float*>(&engine.clearColorValue));
					ImGui::EndPopup();
				}
				//toggle viewport helpers, like grid
				if (ImGui::Button("Grid")) {
					this->gridEnabled = !this->gridEnabled;
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


		// gizmo functionality
		{
			if (this->activeEntity.has_value() && !this->_isCameraControlActive && this->activeEntity.value()->HasComponent<TransformComponent>()) {
				// gizmo frame
				ImGuizmo::BeginFrame();

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
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

				auto& entityTC = this->activeEntity.value()->GetMutableComponent<TransformComponent>();
				// more snapping logic to round position if i start snapping
				glm::mat4 transformMatrix = glm::translate(glm::mat4(1.f), entityTC.Position) *
									  glm::mat4_cast(entityTC.Rotation) *
									  glm::scale(glm::mat4(1.f), entityTC.Scale);

				ImGuizmo::Manipulate(glm::value_ptr(this->camera.GetViewMatrix()),
									 glm::value_ptr(this->camera.GetProjectionMatrix()),
									 _guizmoOperation, _guizmoSpace,
									 glm::value_ptr(transformMatrix), nullptr, snap ? snapValues : nullptr);

				// now apply the transformations
				if (ImGuizmo::IsUsing()) {
					glm::vec3 position;
					glm::quat rotation;
					glm::vec3 scale;

					glm::vec3 skew; // useless
					glm::vec4 perspective; // useless
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
			if (glfwGetMouseButton(this->mainWindow.GetGlfwWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				if (hoveredEntity.has_value() && !ImGuizmo::IsUsing())
					this->activeEntity.emplace(hoveredEntity.value());
			}
			// right now will currently go off in menu that overlaps the image
			if (ImGui::IsMouseDoubleClicked(GLFW_MOUSE_BUTTON_LEFT) && IsMouseInViewportBounds() && !hoveredEntity) {
				this->activeEntity = std::nullopt;
			}
		}
		ImGui::End();
	}
}