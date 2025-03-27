//
// Created by Hayden Rivas on 1/16/25.
//
#include "IconFontCppHeaders/IconsLucide.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <entt/entt.hpp>

#include "Slate/Components.h"
#include "Slate/SceneTemplates.h"

#include "../Editor.h"
#include "../Fonts.h"
#include "../ImGuiSnippets.h"

namespace Slate {

	std::vector<entt::entity> entity_order_vector;
	std::unordered_set<entt::entity> entitySet;

	void Editor::OnScenePanelUpdate() {
		ImGui::Begin("Scene Hierarchy");
		{
			// if we click on any empty space, unselect the entity
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
				this->activeEntity = std::nullopt;


			// right click menu on window
			if (ImGui::BeginPopupContextWindow()) {
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Brighten(ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.2f));
				if (ImGui::Selectable("New Entity"))
					this->activeEntity.emplace(this->scene->CreateEntity("Unnamed Entity"));
				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
			// quick lookups for entity additions and presence in scene
			auto allEntities = this->scene->GetRegistry().view<entt::entity>();
			for (auto entityId : allEntities) {
				if (entitySet.find(entityId) == entitySet.end()) {
					entity_order_vector.push_back(entityId);
					entitySet.insert(entityId);
				}
			}


			int toDeleteIndex = -1;
			// ENTITY LIST
			for (int i = 0; i < entity_order_vector.size(); ++i) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf |
										   ImGuiTreeNodeFlags_NoTreePushOnOpen |
										   ImGuiTreeNodeFlags_FramePadding |
										   ImGuiTreeNodeFlags_SpanFullWidth;
				// from order vector get our entity

				Shared<Entity> entity = this->scene->GetEntityById(entity_order_vector[i]);
				// scene panel actual content
				float ypad = ImGui::GetFontSize() * 0.35f;

				// current entity in vector needs to be the active entity
				bool isSelected = false;
				if (this->activeEntity.has_value()) {
					if (entity->GetHandle() == this->activeEntity.value()->GetHandle()) isSelected = true;
				}
				// if entity is also valid
				if (isSelected && this->activeEntity.has_value()) {
					flags |= ImGuiTreeNodeFlags_Selected;
				}

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ypad));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
				if (isSelected) ImGui::PushStyleColor(ImGuiCol_HeaderHovered,Brighten(ImGui::GetStyleColorVec4(ImGuiCol_Header), 0.035f));
				ImGui::TreeNodeEx((void *) entity->GetHandle(), flags, " " ICON_LC_BOX "  %s", entity->GetName().c_str());
				if (isSelected) ImGui::PopStyleColor();
				ImGui::PopStyleVar(2);
				// dragging and dropping functionality
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
					ImGui::SetDragDropPayload("EntityScenePayload", &i, sizeof(int));
					ImGui::Text("Dragging: %s", entity->GetName().c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					ImVec2 itemRect = ImGui::GetItemRectSize();
					ImVec2 itemRectMin = ImGui::GetItemRectMin();
					ImVec2 itemRectMax = ImGui::GetItemRectMax();

					// determining if we are dropping above or below the hovered item
					float mouseY = ImGui::GetMousePos().y;
					bool dropAbove = (mouseY < (itemRectMin.y + itemRect.y * 0.5f));
					int insertIndex = dropAbove ? i : i + 1;

					ImVec2 barStart = dropAbove ? itemRectMin : ImVec2(itemRectMin.x, itemRectMax.y - itemRectMin.y);
					// lowkey forgot what this was doing compared to barStart above it
					if (i == entity_order_vector.size() - 1) {
						barStart = dropAbove ? itemRectMin : ImVec2(itemRectMin.x, itemRectMax.y);
					}
					ImVec2 barEnd = ImVec2(itemRectMax.x, barStart.y);

					ImDrawList *draw_list = ImGui::GetWindowDrawList();
					draw_list->AddLine(barStart, barEnd, ImGui::GetColorU32(ImGuiCol_DragDropTarget), 3.0f);

					if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EntityScenePayload")) {
						int payloadIndex = *(const int *) payload->Data;
						if (payloadIndex != i) {
							if (payloadIndex < insertIndex) --insertIndex;
							entt::entity draggedEntity = entity_order_vector[payloadIndex];
							entity_order_vector.erase(entity_order_vector.begin() + payloadIndex);
							entity_order_vector.insert(entity_order_vector.begin() + insertIndex, draggedEntity);
						}
					}
					ImGui::EndDragDropTarget();
				}


				// on normal click
				if (ImGui::IsItemClicked())
					this->activeEntity.emplace(entity);
				// right click menu on item, also use the entity id as a id for the popup
				if (ImGui::BeginPopupContextItem(std::to_string((uint64_t) entity->GetHandle()).c_str())) {
					this->activeEntity.emplace(entity);
					// name header
					ImGui::PushFont(Fonts::boldFont);
					ImGui::Text("%s", entity->GetName().c_str());
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
						std::string currentName = entity->GetName();
						strncpy(buffer, currentName.c_str(), sizeof(buffer) - 1);
						buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination
						if (ImGui::InputText("##RenameInput", buffer, IM_ARRAYSIZE(buffer),ImGuiInputTextFlags_EnterReturnsTrue)) {
							entity->SetName(buffer);
							ImGui::ClosePopupsExceptModals();
						}
						ImGui::EndPopup();
					}

					if (ImGui::Selectable("Duplicate")) {
						if (this->activeEntity) {
//							Shared<Entity> newEntity = CreateShared<Entity>(this->scene->DuplicateEntity(this->activeEntity.value()));
//							this->activeEntity.emplace(newEntity); // set newly duplicated entity as active
						}
					}
					ImGui::Selectable("Copy NW");
					ImGui::Selectable("Paste NW");

					ImGui::Separator();
					if (ImGui::Selectable("Delete")) {
						// give entity component which signals its incoming deletion at end of gui update
						toDeleteIndex = i;
						ImGui::CloseCurrentPopup();
					}
					ImGui::PopStyleColor();
					ImGui::EndPopup();
				}
			}
			if (toDeleteIndex != -1) {
				this->activeEntity = std::nullopt;
				this->scene->DestroyEntityById(entity_order_vector[toDeleteIndex]);
				entity_order_vector.erase(entity_order_vector.begin() + toDeleteIndex);
			}
			// end of list
		}
		ImGui::End();
	}
}