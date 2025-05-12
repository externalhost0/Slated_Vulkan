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
	FastVector<entt::entity, MAX_ENTITY_COUNT> user_ordered_handles;
	FastQueue<entt::entity, 16> to_delete_handles;


	// recurse through all children of a node, but only if it would be visible
	void EditorApplication::_displayEntityNode(GameEntity entity) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NoTreePushOnOpen |
								   ImGuiTreeNodeFlags_FramePadding     |
								   ImGuiTreeNodeFlags_SpanFullWidth    |
								   ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (!entity.HasChildren()) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		bool isSelected = false;
		if (this->ctx.activeEntity.has_value()) {
			if (entity == this->ctx.activeEntity.value()) {
				isSelected = true;
				flags |= ImGuiTreeNodeFlags_Selected;
			}
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetFontSize() * 0.35f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		if (isSelected) ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Brighten(ImGui::GetStyleColorVec4(ImGuiCol_Header), 0.035f));
		bool opened = ImGui::TreeNodeEx((void *) entity.GetHandle(), flags, " " ICON_LC_BOX "  %s", entity.GetName().c_str());
		if (isSelected) ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		if (ImGui::IsItemClicked()) {
			this->ctx.activeEntity.emplace(entity);
		}

		if (ImGui::BeginPopupContextItem(std::to_string((uint64_t) entity.GetHandle()).c_str())) {
			this->ctx.activeEntity.emplace(entity);
			// name header
			ImGui::PushFont(Fonts::boldFont);
			ImGui::Text("%s", entity.GetName().c_str());
			ImGui::PopFont();
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
								  Brighten(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.2f));
			// rename functionality
			if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_NoAutoClosePopups))
				ImGui::OpenPopup("RenamePopup"); // Open the popup when button is clicked

			if (ImGui::BeginPopup("RenamePopup")) {
				ImGui::SetKeyboardFocusHere();
				// buffer m_Count and actual name
				// currently max m_Count name is 64
				char buffer[64] = "";
				std::string currentName = entity.GetName();
				strncpy(buffer, currentName.c_str(), sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination
				if (ImGui::InputText("##RenameInput", buffer, IM_ARRAYSIZE(buffer),ImGuiInputTextFlags_EnterReturnsTrue)) {
					entity.SetName(buffer);
					ImGui::ClosePopupsExceptModals();
				}
				ImGui::EndPopup();
			}

			if (ImGui::Selectable("Duplicate")) {
				GameEntity duped_entity = this->ctx.scene->DuplicateEntity(this->ctx.activeEntity.value());
				// set newly duplicated entity as active
				this->ctx.activeEntity.emplace(duped_entity);
			}
			ImGui::Selectable("Copy NW");
			ImGui::Selectable("Paste NW");
			ImGui::Separator();
			if (ImGui::Selectable("Delete")) {
				this->ctx.activeEntity = std::nullopt;
				to_delete_handles.push(entity.GetHandle());

				ImGui::CloseCurrentPopup();
				ImGui::PopStyleColor();
				ImGui::EndPopup();
				return;
			}
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		// recurse children
		if (!opened) return;
		for (GameEntity child : entity.GetChildren()) {
			_displayEntityNode(child);
		}
	}

//	void Editor::DrawEntityNode(Entity* entity, int i) {
//		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NoTreePushOnOpen |
//								   ImGuiTreeNodeFlags_FramePadding |
//								   ImGuiTreeNodeFlags_SpanFullWidth;
//		if (!entity->HasChildren()) {
//			flags |= ImGuiTreeNodeFlags_Leaf;
//		}
//		// scene panel actual content
//		float ypad = ImGui::GetFontSize() * 0.35f;
//
//		// current entity in vector needs to be the active entity
//		bool isSelected = false;
//		if (this->activeEntity.has_value()) {
//			if (entity->GetHandle() == this->activeEntity.value()->GetHandle()) isSelected = true;
//		}
//		// if entity is also valid
//		if (isSelected && this->activeEntity.has_value()) {
//			flags |= ImGuiTreeNodeFlags_Selected;
//		}
//
//		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ypad));
//		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
//		if (isSelected) ImGui::PushStyleColor(ImGuiCol_HeaderHovered,Brighten(ImGui::GetStyleColorVec4(ImGuiCol_Header), 0.035f));
//		bool opened = ImGui::TreeNodeEx((void *) entity->GetHandle(), flags, " " ICON_LC_BOX "  %s", entity->GetName().c_str());
//		if (isSelected) ImGui::PopStyleColor();
//		ImGui::PopStyleVar(2);
//		// dragging and dropping functionality // ImGuiDragDropFlags_AcceptNoDrawDefaultRect
//		if (ImGui::BeginDragDropSource()) {
//			ImGui::SetDragDropPayload("EntityScenePayload", &i, sizeof(int));
//			ImGui::Text("Dragging: %s", entity->GetName().c_str());
//			ImGui::EndDragDropSource();
//		}
//
//		if (ImGui::BeginDragDropTarget()) {
//			ImVec2 itemRect = ImGui::GetItemRectSize();
//			ImVec2 itemRectMin = ImGui::GetItemRectMin();
//			ImVec2 itemRectMax = ImGui::GetItemRectMax();
//
//			// determining if we are dropping above or below the hovered item
//			float mouseY = ImGui::GetMousePos().y;
//			bool dropAbove = (mouseY < (itemRectMin.y + itemRect.y * 0.5f));
//			int insertIndex = dropAbove ? i : i + 1;
//
//			ImVec2 barStart = dropAbove ? itemRectMin : ImVec2(itemRectMin.x, itemRectMax.y - itemRectMin.y);
//			// lowkey forgot what this was doing compared to barStart above it
//			if (i == entity_order_vector.size() - 1) {
//				barStart = dropAbove ? itemRectMin : ImVec2(itemRectMin.x, itemRectMax.y);
//			}
//			ImVec2 barEnd = ImVec2(itemRectMax.x, barStart.y);
//
//			ImDrawList *draw_list = ImGui::GetWindowDrawList();
//			draw_list->AddLine(barStart, barEnd, ImGui::GetColorU32(ImGuiCol_DragDropTarget), 3.0f);
//
//			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EntityScenePayload")) {
//				int payloadIndex = *(const int *) payload->Data;
//				if (payloadIndex != i) {
//					if (payloadIndex < insertIndex) --insertIndex;
//					entt::entity draggedEntity = entity_order_vector[payloadIndex];
//					entity_order_vector.erase(entity_order_vector.begin() + payloadIndex);
//					entity_order_vector.insert(entity_order_vector.begin() + insertIndex, draggedEntity);
//				}
//			}
//			ImGui::EndDragDropTarget();
//		}
//
////		if (opened) {
////			for (const StrongPtr<Entity>& child : entity->GetChildrenHandles()) {
////				DrawEntityNode(child.get(), i);
////			}
////		}
//
//	}

	void EditorApplication::_onScenePanelUpdate() {
		ImGui::Begin("Scene Hierarchy");
		{
			// if we click on any empty space, unselect the entity
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
				this->ctx.activeEntity = std::nullopt;
			}

			// right click menu on window
			if (ImGui::BeginPopupContextWindow()) {
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Brighten(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.2f));
				if (ImGui::Selectable("New Entity")) {
					this->ctx.activeEntity.emplace(this->ctx.scene->CreateEntity("Unnamed Enitty"));
				}
				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
			// quick lookups for entity additions and presence in scene
			for (GameEntity entity : this->ctx.scene->GetRootEntities()) {
				const entt::entity handle = entity.GetHandle();
				if (std::find(user_ordered_handles.begin(), user_ordered_handles.end(), handle) == user_ordered_handles.end()) {
					user_ordered_handles.push_back(handle);
				}
			}

			// recurse through all top level entities
			// when entities have no children they are leaves
			for (entt::entity handle : user_ordered_handles) {
				// from order vector get our entity
				_displayEntityNode(this->ctx.scene->resolveEntity(handle));
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