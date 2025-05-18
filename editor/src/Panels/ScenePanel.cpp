//
// Created by Hayden Rivas on 1/16/25.
//
#include "Slate/ECS/Components.h"
#include <Slate/SceneTemplates.h>

#include "../Editor.h"
#include "../ImGuiSnippets.h"

#include <iostream>
#include <entt/entt.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "IconFontCppHeaders/IconsLucide.h"

namespace Slate {
	std::vector<entt::entity> user_ordered_handles;
	FastQueue<entt::entity, 16> to_delete_handles;


//	void EditorApplication::_displayEntityNodeOld(GameEntity entity, int index) {
//		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NoTreePushOnOpen |
//								   ImGuiTreeNodeFlags_FramePadding     |
//								   ImGuiTreeNodeFlags_SpanFullWidth    |
//								   ImGuiTreeNodeFlags_OpenOnDoubleClick;
//		if (!entity.HasChildren()) {
//			flags |= ImGuiTreeNodeFlags_Leaf;
//		}
//
//		bool isSelected = false;
//		if (this->ctx.activeEntity.has_value()) {
//			if (entity == this->ctx.activeEntity.value()) {
//				isSelected = true;
//				flags |= ImGuiTreeNodeFlags_Selected;
//			}
//		}
//
//		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetFontSize() * 0.35f));
//		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
//		if (isSelected) ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Brighten(ImGui::GetStyleColorVec4(ImGuiCol_Header), 0.035f));
//		bool opened = ImGui::TreeNodeEx((void *) entity.GetHandle(), flags, " " ICON_LC_BOX "  %s", entity.GetName().c_str());
//		if (isSelected) ImGui::PopStyleColor();
//		ImGui::PopStyleVar(2);
//
//		if (ImGui::IsItemClicked()) {
//			this->ctx.activeEntity.emplace(entity);
//		}
//
//		// dragging source
//		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
//			ImGui::SetDragDropPayload("EntityScenePayload", &index, sizeof(int));  // Payload: the index of the entity in the vector
//			ImGui::Text("Dragging: %s", entity.GetName().c_str());  // Displaying the name of the entity being dragged
//			ImGui::EndDragDropSource();
//		}
//		// drop target
//		if (ImGui::BeginDragDropTarget()) {
//			ImVec2 itemRect = ImGui::GetItemRectSize();
//			ImVec2 itemRectMin = ImGui::GetItemRectMin();
//			ImVec2 itemRectMax = ImGui::GetItemRectMax();
//
//			// determine if above or below item
//			float mouseY = ImGui::GetMousePos().y;
//			bool dropAbove = (mouseY < (itemRectMin.y + itemRect.y * 0.5f));
//			int insertIndex = dropAbove ? index : index + 1;  // If above, drop before, else after
//
//			// line indication of drop target
//			ImVec2 barStart = dropAbove ? itemRectMin : ImVec2(itemRectMin.x, itemRectMax.y);
//			ImVec2 barEnd = ImVec2(itemRectMax.x, barStart.y);
//			ImDrawList* draw_list = ImGui::GetWindowDrawList();
//			draw_list->AddLine(barStart, barEnd, ImGui::GetColorU32(ImGuiCol_DragDropTarget), 3.0f);
//
//			// reordering logic
//			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityScenePayload")) {
//				int payloadIndex = *(const int*)payload->Data; // index inside vector of handles
//				if (payloadIndex != index) {
//					// If the dragged entity is not the same as the target, we need to adjust the insert index
//					if (payloadIndex < insertIndex) --insertIndex;
//					// Remove the dragged entity and insert it at the new position
//					entt::entity draggedEntity = user_ordered_handles[payloadIndex];
//					user_ordered_handles.erase(user_ordered_handles.begin() + payloadIndex);
//					user_ordered_handles.insert(user_ordered_handles.begin() + insertIndex, draggedEntity);
//				}
//			}
//
//			ImGui::EndDragDropTarget();
//		}
//
//
//		// popup menu
//		if (ImGui::BeginPopupContextItem(std::to_string((uint64_t) entity.GetHandle()).c_str())) {
//			this->ctx.activeEntity.emplace(entity);
//			// name header
//			ImGui::PushFont(Fonts::boldFont);
//			ImGui::Text("%s", entity.GetName().c_str());
//			ImGui::PopFont();
//			ImGui::Separator();
//
//			ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
//								  Brighten(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.2f));
//			// rename functionality
//			if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_NoAutoClosePopups))
//				ImGui::OpenPopup("RenamePopup"); // open the popup when button is clicked
//
//			if (ImGui::BeginPopup("RenamePopup")) {
//				ImGui::SetKeyboardFocusHere();
//				// buffer m_Count and actual name
//				// currently max m_Count name is 64
//				char buffer[64] = "";
//				const std::string& currentName = entity.GetName();
//				strncpy(buffer, currentName.c_str(), sizeof(buffer) - 1);
//				buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination
//				if (ImGui::InputText("##RenameInput", buffer, IM_ARRAYSIZE(buffer),ImGuiInputTextFlags_EnterReturnsTrue)) {
//					entity.SetName(buffer);
//					ImGui::ClosePopupsExceptModals();
//				}
//				ImGui::EndPopup();
//			}
//
//			if (ImGui::Selectable("Duplicate")) {
//				GameEntity duped_entity = this->ctx.scene->DuplicateEntity(this->ctx.activeEntity.value());
//				// set newly duplicated entity as active
//				this->ctx.activeEntity.emplace(duped_entity);
//			}
//			ImGui::Selectable("Copy NW");
//			ImGui::Selectable("Paste NW");
//			ImGui::Separator();
//			if (ImGui::Selectable("Delete")) {
//				this->ctx.activeEntity = std::nullopt;
//				to_delete_handles.push(entity.GetHandle());
//
//				ImGui::CloseCurrentPopup();
//				ImGui::PopStyleColor();
//				ImGui::EndPopup();
//				return;
//			}
//			ImGui::PopStyleColor();
//			ImGui::EndPopup();
//		}
//
//		// recurse children
//		if (!opened) return;
//		for (GameEntity child: entity.GetChildren()) {
//			ImGui::Indent(15.f);
//			_displayEntityNode(child, index);
//			ImGui::Unindent(15.f);
//		}
//	}

	void EditorApplication::_displayEntityNode(GameEntity entity, int index) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NoTreePushOnOpen |
								   ImGuiTreeNodeFlags_FramePadding |
								   ImGuiTreeNodeFlags_SpanFullWidth |
								   ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (!entity.hasChildren()) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		bool isSelected = false;
		if (this->ctx.activeEntity.has_value()) {
			if (entity == this->ctx.activeEntity.value()) {
				isSelected = true;
				flags |= ImGuiTreeNodeFlags_Selected;
			}
		}

		float ypad = ImGui::GetFontSize() * 0.35f;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ypad));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		if (isSelected) ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Brighten(ImGui::GetStyleColorVec4(ImGuiCol_Header), 0.035f));

		bool opened = ImGui::TreeNodeEx((void*) entity.getHandle(), flags, " " ICON_LC_BOX "  %s", entity.getName().c_str());
		if (isSelected) ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		if (ImGui::IsItemClicked()) {
			this->ctx.activeEntity.emplace(entity);
		}

		// drag source
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
			ImGui::SetDragDropPayload("EntityScenePayload", &index, sizeof(int)); // index, and parent's index inside root handle vector
			ImGui::Text("Dragging: %s", entity.getName().c_str());
			ImGui::EndDragDropSource();
		}

		// drop target
		if (ImGui::BeginDragDropTarget()) {
			ImVec2 itemRect = ImGui::GetItemRectSize();
			ImVec2 itemRectMin = ImGui::GetItemRectMin();
			ImVec2 itemRectMax = ImGui::GetItemRectMax();

			// determining if we should drop above, below or inside (child)
			float mouseY = ImGui::GetMousePos().y;
			bool dropAbove = (mouseY < (itemRectMin.y + itemRect.y * 0.33f));
			bool dropBelow = (mouseY > (itemRectMax.y - itemRect.y * 0.33f));
			bool dropInCenter = !dropAbove && !dropBelow;

			int insertIndex = dropAbove ? index : (dropBelow ? index + 1 : index);

			ImVec2 barStart = dropInCenter ? itemRectMin : (dropAbove ? itemRectMin : ImVec2(itemRectMin.x, itemRectMax.y));
			ImVec2 barEnd = ImVec2(itemRectMax.x, barStart.y);
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// if the drop is in the center highlight the entire item
			// otherwise highlight the drop target line (top or bottom)
			ImVec4 dragcolor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_DragDropTarget));
			if (dropInCenter) {
				draw_list->AddRectFilled(itemRectMin, itemRectMax, ImColor(dragcolor.x, dragcolor.y, dragcolor.z, 0.2f), 3.0f);
			} else {
				draw_list->AddLine(barStart, barEnd, ImGui::GetColorU32(ImGuiCol_DragDropTarget), 3.0f);
			}

			// dropped logic
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityScenePayload")) {
				int payloadIndex = *(const int*)payload->Data;
				GameEntity payloadGameEntity = this->ctx.scene->resolveEntity(user_ordered_handles[payloadIndex]);
				fmt::print("{}\n", payloadIndex);
				if (payloadIndex != index) {
					if (dropInCenter) {
						// if drop is on entity
						if (payloadGameEntity.hasParent()) {
							payloadGameEntity.getParent().removeChild(entity);
							entity.removeChild(payloadGameEntity);
						} else {
							user_ordered_handles.erase(user_ordered_handles.begin() + payloadIndex);
							entity.addChild(payloadGameEntity);
						}
					} else {
						// if drop is beside/between entity
						if (payloadIndex < insertIndex) --insertIndex;
						user_ordered_handles.erase(user_ordered_handles.begin() + payloadIndex);
						user_ordered_handles.insert(user_ordered_handles.begin() + insertIndex, payloadGameEntity.getHandle());
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		// popup menu
		if (ImGui::BeginPopupContextItem(std::to_string((uint64_t) entity.getHandle()).c_str())) {
			this->ctx.activeEntity.emplace(entity);
			// name header
			ImGui::PushFont(Fonts::boldFont);
			ImGui::Text("%s", entity.getName().c_str());
			ImGui::PopFont();
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
								  Brighten(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.2f));
			// rename functionality
			if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_NoAutoClosePopups))
				ImGui::OpenPopup("RenamePopup");

			if (ImGui::BeginPopup("RenamePopup")) {
				ImGui::SetKeyboardFocusHere();
				char buffer[64] = "";
				const std::string& currentName = entity.getName();
				strncpy(buffer, currentName.c_str(), sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination
				if (ImGui::InputText("##RenameInput", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
					entity.setName(buffer);
					ImGui::ClosePopupsExceptModals();
				}
				ImGui::EndPopup();
			}

			if (ImGui::Selectable("Duplicate")) {
				GameEntity duped_entity = this->ctx.scene->DuplicateEntity(this->ctx.activeEntity.value());
				this->ctx.activeEntity.emplace(duped_entity);
			}
			ImGui::Selectable("Copy NW");
			ImGui::Selectable("Paste NW");
			ImGui::Separator();
			if (ImGui::Selectable("Delete")) {
				this->ctx.activeEntity = std::nullopt;
				to_delete_handles.push(entity.getHandle());

				ImGui::CloseCurrentPopup();
				ImGui::PopStyleColor();
				ImGui::EndPopup();
				return;
			}
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		if (opened) {
			for (GameEntity child : entity.getChildren()) {
				ImGui::Indent(15.f);
				_displayEntityNode(child, index);
				ImGui::Unindent(15.f);
			}
		}
	}


	void EditorApplication::_onScenePanelUpdate() {
		ImGui::Begin("Scene Hierarchy");
		{
			// if we click on any empty space, unselect the entity
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
				this->ctx.activeEntity = std::nullopt;
			}

			// right click menu on window empty space
			if (ImGui::BeginPopupContextWindow()) {
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Brighten(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.2f));
				if (ImGui::Selectable("New Entity")) {
					this->ctx.activeEntity.emplace(this->ctx.scene->createEntity("Unnamed Enitty"));
				}
				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
			// quick lookups for entity additions and presence in scene
			for (GameEntity entity : this->ctx.scene->GetRootEntities()) {
				const entt::entity handle = entity.getHandle();
				if (std::find(user_ordered_handles.begin(), user_ordered_handles.end(), handle) == user_ordered_handles.end()) {
					user_ordered_handles.push_back(handle);
				}
			}
			// recurse through all top level entities
			// when entities have no children they are leaves
			int i = 0;
			for (entt::entity handle : user_ordered_handles) {
				// from order vector get our entity
				_displayEntityNode(this->ctx.scene->resolveEntity(handle), i);
				i++;
			}
			// deletion queue after rendering
			while (!to_delete_handles.empty()) {
				entt::entity handle = to_delete_handles.front();
				this->ctx.scene->DestroyEntity(handle);

				auto it = std::find(user_ordered_handles.begin(), user_ordered_handles.end(), handle);
				if (it != user_ordered_handles.end()) {
					user_ordered_handles.erase(it);
				}
				to_delete_handles.pop();
			}
		}
		ImGui::End();
	}
}