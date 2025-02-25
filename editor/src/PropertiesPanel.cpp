//
// Created by Hayden Rivas on 1/16/25.
//
#include <Slate/Components.h>

#include <IconFontCppHeaders/IconsLucide.h>

#include <span>
#include <imgui.h>
#include <imgui_internal.h>

#include "Context.h"
#include "EditorGui.h"
#include "Fonts.h"
#include "ImGuiSnippets.h"
#include "ImGuiComponents.h"

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
		auto& transform = entity->GetComponent<TransformComponent>();

		Vector3Drag("Position", transform.Position, "%.2f", 0.0f, 0.01f);

		glm::vec3 intermediate = glm::degrees(glm::eulerAngles(transform.Rotation));
		Vector3Drag("Rotation", intermediate, "%.1f", 0.0f, 0.1f);
		transform.Rotation = glm::normalize(glm::quat(glm::radians(intermediate)));

		Vector3Drag("Scale", transform.Scale, "%.3f", 1.0f, 0.01f);
	}
	void ComponentMesh(Entity* entity) {
		auto& component = entity->GetComponent<MeshComponent>();

		ImGui::Text("%s", component.shaderProgram.filename.c_str());

//		SlateGui::ColorField();
		for (const auto& block : component.shaderProgram.uniformBlockObjects) {
			ImGui::PushFont(Fonts::boldFont);
			ImGui::Text("%s", block.name.c_str());
			ImGui::PopFont();

			for (const auto& member : block.members) {
				if (std::holds_alternative<CustomType>(member)) {
					CustomType type = std::get<CustomType>(member);

					ImGui::Text("Name: %s", type.name.c_str());
					ImGui::Text("Type: %s", type.usertype.c_str());

				} else {
					PlainType type = std::get<PlainType>(member);

					ImGui::Text("Name: %s", type.name.c_str());
					ImGui::Text("Type: %s", Util::StringFromEngineType(type.glsltype).c_str());
				}
				ImGui::NewLine();
			}
		}

	}
	void ComponentScript(Entity* entity) {

	}

	ImVec4 BlackbodyToRGB(float temperature) {
		float t = temperature / 1000.0f; // Normalize to range

		float r, g, b;

		// Red Channel
		if (t <= 1.0f) r = 1.0f;
		else if (t < 2.0f) r = 1.0f;
		else r = 1.0f, r = 329.698727446 * pow(t - 1.0, -0.1332047592);
		r = std::clamp(r / 255.0f, 0.0f, 1.0f);

		// Green Channel
		if (t <= 0.66f) g = 0.0f;
		else if (t <= 2.0f) g = 99.4708025861 * log(t) - 161.1195681661;
		else g = 288.1221695283 * pow(t - 0.6, -0.0755148492);
		g = std::clamp(g / 255.0f, 0.0f, 1.0f);

		// Blue Channel
		if (t >= 0.66f) b = 1.0f;
		else if (t <= 0.19f) b = 0.0f;
		else b = 138.5177312231 * log(t - 0.1) - 305.0447927307;
		b = std::clamp(b / 255.0f, 0.0f, 1.0f);

		return {r, g, b, 1.0};
	}

	void ComponentPointLight(Entity* entity) {
		auto& light = entity->GetComponent<PointLightComponent>();

		SlateGui::ColorField(light.point.Color);
		ImGui::DragFloat("Intensity", &light.point.Intensity, 0.05f, 0.0f, 1000.0f);
		ImGui::DragFloat("Range", &light.point.Range, 0.5f, 0.0f, 100.0f);
	}
	void ComponentSpotLight(Entity* entity) {
		auto& light = entity->GetComponent<SpotLightComponent>();

		SlateGui::ColorField(light.spot.Color);
		ImGui::DragFloat("Intensity", &light.spot.Intensity, 0.05f, 0.0f, 1000.0f);
	}

	void ComponentDirectionalLight(Entity* entity) {
		auto& light = entity->GetComponent<DirectionalLightComponent>();

		SlateGui::ColorField(light.directional.Color);
		ImGui::DragFloat("Intensity", &light.directional.Intensity, 0.05f, 0.0f, 1000.0f);
	}
	void ComponentEnvironmentLight(Entity* entity) {
		// ambient point source changes
		auto& light = entity->GetComponent<EnvironmentComponent>();

		SlateGui::ColorField(light.ambient.Color);
		ImGui::DragFloat("Intensity", &light.ambient.Intensity, 0.05f, 0.0f, 1000.0f);
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
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap;

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
		if (pActiveContext->entity.has_value()) {

			Entity& entity = pActiveContext->entity.value();// alias active entity
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
	template <>
	struct ComponentMetadata<PointLightComponent> {
		static constexpr const char* name = "Point Light";
		static constexpr const char* icon = ICON_LC_LIGHTBULB;
		static constexpr auto handler = ComponentPointLight;
	};
	template <>
	struct ComponentMetadata<DirectionalLightComponent> {
		static constexpr const char* name = "Directional Light";
		static constexpr const char* icon = ICON_LC_SUN;
		static constexpr auto handler = ComponentDirectionalLight;
	};
	template <>
	struct ComponentMetadata<EnvironmentComponent> {
		static constexpr const char* name = "Environment Light";
		static constexpr const char* icon = ICON_LC_GLOBE;
		static constexpr auto handler = ComponentEnvironmentLight;
	};
	template <>
	struct ComponentMetadata<SpotLightComponent> {
		static constexpr const char* name = "Spot Light";
		static constexpr const char* icon = ICON_LC_LAMP_CEILING;
		static constexpr auto handler = ComponentSpotLight;
	};

}