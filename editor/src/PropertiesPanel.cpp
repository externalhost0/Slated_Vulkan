//
// Created by Hayden Rivas on 1/16/25.
//
#include <Slate/Components.h>

#include <IconFontCppHeaders/IconsLucide.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Context.h"
#include "EditorGui.h"
#include "Fonts.h"
#include "ImGuiSnippets.h"

namespace Slate {
	void ComponentCore(Entity& entity) {
		char buffer[128] = "";
		std::string currentName = entity.GetComponent<CoreComponent>().name;
		strncpy(buffer, currentName.c_str(), sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination


		ImGui::PushFont(Fonts::iconLargeFont);
		ImGui::Text(ICON_LC_BOX);
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
		ImGui::PushFont(Fonts::largeboldFont);
		if (ImGui::InputText("##RenameInput", buffer, IM_ARRAYSIZE(buffer),ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
			entity.GetComponent<CoreComponent>().name = buffer;
		}
		ImGui::PopFont();
	}
	void ComponentTransform(Entity* entity) {
		auto &transform = entity->GetComponent<TransformComponent>();

		Vector3Drag("Position", transform.Position, "%.2f", 0.0f, 0.01f);

		glm::vec3 intermediate = glm::degrees(glm::eulerAngles(transform.Rotation));
		Vector3Drag("Rotation", intermediate, "%.1f", 0.0f, 0.1f);
		transform.Rotation = glm::normalize(glm::quat(glm::radians(intermediate)));

		Vector3Drag("Scale", transform.Scale, "%.3f", 1.0f, 0.01f);
	}
	void ComponentMesh(Entity* entity) {

	}
	void ComponentScript(Entity* entity) {

	}
	template <typename T>
	struct ComponentMetadata { static_assert(sizeof(T) == 0); };

	template <typename... Components>
	void ProcessComponents(Entity* entity, ComponentGroup<Components...>) {
		(..., [&] {
			if (entity->HasComponent<Components>()) {
				ComponentPropertyPanel<Components>(
						entity,
						ComponentMetadata<Components>::handler,
						ComponentMetadata<Components>::name,
						ComponentMetadata<Components>::icon
				);
			}
		}());
	}
	// templated function for all panels
	template <typename ComponentType>
	void ComponentPropertyPanel(Entity* entity, const std::function<void(Entity*)>& func, const char* title, const char* icon) {
		// make sure a component is only shown if an entity has it
		if (!entity || !entity->HasComponent<ComponentType>()) return;
		// require that the componenttype is actually a component lol
		static_assert(std::is_class<ComponentType>::value, "ComponentType must be a class/struct!");

		// some flags we want for the following imgui elements
		ImGuiChildFlags childFlags = ImGuiChildFlags_AutoResizeY;
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap;

		// generate a unique id for the child window
		std::string uniqueId = "Child" + std::to_string(typeid(ComponentType).hash_code()) + std::to_string(static_cast<uint32_t>(entity->GetHandle()));
		ImGui::BeginChild(uniqueId.c_str(), ImVec2(0, 0), childFlags);

		// this constructs the icon -> title -> and set tree visibillity
		ImGui::PushFont(Fonts::boldFont);
		float cursorY = ImGui::GetCursorPosY();
		bool isOpen = ImGui::TreeNodeEx((void*) typeid(ComponentType).hash_code(), nodeFlags, "%s", icon);
		ImGui::SetCursorPosY(cursorY);
		float pad = ImGui::GetFontSize() * 4.2f;
		ImGui::SameLine(pad);
		ImGui::Text("%s", title);
		ImGui::PopFont();

		bool remove = false;
		const char* label = ICON_LC_TRASH;
		float space_available = ImGui::GetContentRegionAvail().x;
		if (space_available > ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2) {
			ImGui::SameLine(ImGui::GetCursorPosX() + space_available - ImGui::CalcTextSize(label).x - ImGui::GetStyle().FramePadding.x * 2);

			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.961, 0.357, 0.357, 0.7f));
			if (ImGui::Button(label)) {
				remove = true;
			}
			ImGui::PopStyleColor();
		}
		// where your passed function will go
		if (isOpen){
			func(entity); // run the custom property gui you gave
			ImGui::TreePop();
		}
		// remove at end of call
		if (remove) {
			entity->RemoveComponent<ComponentType>();
			if (std::is_same_v<ComponentType, TransformComponent>) {
				entity = nullptr;
			}
		}
		ImGui::EndChild();
	}

	template <typename... Component>
	void ProcessAllComponents(Entity& entity, ComponentGroup<Component...>) {
		(..., [&] {
			bool hasComponent = entity.HasComponent<Component>();
			if (hasComponent) {
				ImGui::BeginDisabled();
			}
			const char *componentName = ComponentMetadata<Component>::name;
			if (ImGui::Selectable(componentName)) {
				entity.AddComponent<Component>();
			}
			if (hasComponent) {
				ImGui::EndDisabled();
			}
		}());
	}

	void EditorGui::OnPropertiesPanelUpdate() {
		ImGui::Begin("Properties");
		if (pActiveContext->entity) {

			Entity& entity = pActiveContext->entity;// alias active entity
			ComponentCore(entity);
			ImGui::Separator();
			ImGui::Spacing();
			ProcessComponents(&entity, AllComponents{});

			// add comp button at bottom
			const char* label = "Add Component";
			float availableWidth = ImGui::GetContentRegionAvail().x;
			ImGui::SetCursorPosX((availableWidth - ImGui::CalcTextSize(label).x) / 2.0f);
			if (ImGui::Button(label)) {
				ImGui::OpenPopup("ComponentList");
			}
			if(ImGui::BeginPopup("ComponentList")) {
				ProcessAllComponents(entity, AllComponents{});
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}



	template <>
	struct ComponentMetadata<TransformComponent> {
		static constexpr const char* name = "Transform";
		static constexpr const char* icon = ICON_LC_MOVE_3D;
		static constexpr auto handler = ComponentTransform;
	};
	template <>
	struct ComponentMetadata<ScriptComponent> {
		static constexpr const char* name = "Script";
		static constexpr const char* icon = ICON_LC_SCROLL;
		static constexpr auto handler = ComponentScript;
	};
	template <>
	struct ComponentMetadata<MeshComponent> {
		static constexpr const char* name = "Mesh";
		static constexpr const char* icon = ICON_LC_BLOCKS;
		static constexpr auto handler = ComponentMesh;
	};

}