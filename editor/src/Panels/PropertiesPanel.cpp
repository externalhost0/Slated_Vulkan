//
// Created by Hayden Rivas on 1/16/25.
//
#include "Slate/Components.h"

#include "IconFontCppHeaders/IconsLucide.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <span>

#include "../Editor.h"
#include "../Fonts.h"
#include "../ImGuiComponents.h"
#include "../ImGuiSnippets.h"

namespace Slate {
	void EntityHeader(Entity& entity) {
		char buffer[128] = "";
		std::string currentName = entity.GetName();
		strncpy(buffer, currentName.c_str(), sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

		ImGui::PushFont(Fonts::iconLargeFont);
		ImGui::Text(ICON_LC_BOX);
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
		ImGui::PushFont(Fonts::largeboldFont);
		if (ImGui::InputText("##RenameInput", buffer, IM_ARRAYSIZE(buffer),ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
			entity.SetName(buffer);
		}
		ImGui::PopFont();
	}
	void ComponentTransform(Entity& entity) {
		auto& transform = entity.GetComponent<TransformComponent>();

		Vector3Drag("Position", transform.position, "%.2f", 0.0f, 0.01f);

		glm::vec3 intermediate = glm::degrees(glm::eulerAngles(transform.rotation));
		Vector3Drag("Rotation", intermediate, "%.1f", 0.0f, 0.1f);
		transform.rotation = glm::normalize(glm::quat(glm::radians(intermediate)));

		Vector3Drag("Scale", transform.scale, "%.3f", 1.0f, 0.01f);
	}
	void ComponentMeshPrimitive(Entity& entity) {
		GeometryPrimitiveComponent& mesh_component = entity.GetComponent<GeometryPrimitiveComponent>();
		MeshPrimitiveType current_type = mesh_component.mesh_type;

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Mesh Type: ");
		ImGui::SameLine();
		if (ImGui::BeginCombo("###hidden", MeshPrimitiveTypeStringMap.at(current_type))) {
			for (const auto& [type, name] : MeshPrimitiveTypeStringMap) {
				bool isSelected = (current_type == type);
				if (ImGui::Selectable(name, isSelected)) {
					mesh_component.mesh_type = type;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

//		std::string shader_name = mesh_component.shader->GetName();
//		ImGui::Text("%s", shader_name.c_str());
//		for (const Uniform& uniform : mesh_component.uniforms) {
//			ImGui::Text("%s", uniform.name.c_str());
//			if (uniform.type == ShaderType::Vec3) {
//				ImGui::DragFloat3("#hiddenitem", glm::value_ptr(*static_cast<glm::vec3*>(uniform.data)));
//			}
//		}

//		ImGui::Text("%s", mesh.shaderProgram.filename.c_str());

//		SlateGui::ColorField();

//		for (size_t k = 1; k < mesh.shaderProgram.descriptorSets.size(); ++k) {
//			DescriptorSet& set = mesh.shaderProgram.descriptorSets[k];
//			for (UBO& block : set.uniformBlockObjects) {
//				if (!block.exposed) continue;
//				ImGui::PushFont(Fonts::boldFont);
//				ImGui::Text("%s", block.name.c_str());
//				ImGui::PopFont();
//
//				for (auto& member : block.members) {
//					if (std::holds_alternative<CustomType>(member)) {
//						auto& type = std::get<CustomType>(member);
//
//						ImGui::Text("Name: %s", type.name.c_str());
//						ImGui::Text("Type: %s", type.usertype.c_str());
//					} else {
//						auto& type = std::get<PlainType>(member);
//
//						ImGui::Text("Name: %s", type.name.c_str());
//						ImGui::Text("Type: %s", Util::StringFromEngineType(type.glsltype).c_str());
//						if (type.glsltype == ShaderType::Vec3) {
//							auto& vec = std::get<glm::vec3>(type.value);
//							ImGui::DragFloat3("te", glm::value_ptr(vec));
//						}
//					}
//					ImGui::NewLine();
//				}
//
//			}
//		}
	}
	void ComponentMeshGLTF(Entity& entity) {

	}
	void ComponentScript(Entity& entity) {

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

	float largest_width = 95.f;
	float sliderWidth = 150.f;
	void ComponentILight(glm::vec3& color, float& intensity) {
		ImGui::Text("Color");
		ImGui::SameLine(largest_width);
		SlateGui::ColorField(color);

		ImGui::Text("Intensity");
		ImGui::SameLine(largest_width);
		ImGui::PushItemWidth(sliderWidth);
		ImGui::DragFloat("###hiddenintense", &intensity, 0.05f, 0.f, 1000.0f);
		ImGui::PopItemWidth();
	}
	void ComponentPointLight(Entity& entity) {
		auto& light = entity.GetComponent<PointLightComponent>();
		ComponentILight(light.point.Color, light.point.Intensity);

		ImGui::Text("Range");
		ImGui::SameLine(largest_width);
		ImGui::PushItemWidth(sliderWidth);
		ImGui::DragFloat("###hiddenrange", &light.point.Range, 0.5f, 0.f, 100.0f);
		ImGui::PopItemWidth();
	}
	void ComponentSpotLight(Entity& entity) {
		auto& light = entity.GetComponent<SpotLightComponent>();
		ComponentILight(light.spot.Color, light.spot.Intensity);

		ImGui::Text("Blend");
		ImGui::SameLine(largest_width);
		ImGui::PushItemWidth(sliderWidth);
		ImGui::DragFloat("###hiddenblend", &light.spot.Blend, 0.01f, 0.f, 1.f);
		ImGui::PopItemWidth();

		ImGui::Text("Size");
		ImGui::SameLine(largest_width);
		ImGui::PushItemWidth(sliderWidth);
		ImGui::DragFloat("###hiddensize", &light.spot.Size, 0.5f, 1.f, 180.f);
		ImGui::PopItemWidth();
	}
	void ComponentDirectionalLight(Entity& entity) {
		auto& light = entity.GetComponent<DirectionalLightComponent>();
		ComponentILight(light.directional.Color, light.directional.Intensity);
	}
	void ComponentEnvironmentLight(Entity& entity) {
		auto& light = entity.GetComponent<AmbientLightComponent>();
		ComponentILight(light.ambient.Color, light.ambient.Intensity);
	}


	// ensure every component has at least something to display in panel
	template <typename T>
	struct ComponentMetadata { static_assert(sizeof(T) == 0); };

	template <typename... T>
	void ProcessComponents(Entity& entity, ComponentGroup<T...>) {
		(..., [&] {
			if (entity.HasComponent<T>()) {
				ComponentPropertyPanel<T>(
						entity,
						ComponentMetadata<T>::handler,
						ComponentMetadata<T>::name,
						ComponentMetadata<T>::icon
				);
			}
		}());
	}
	// templated function for all component panels
	template <typename T>
	void ComponentPropertyPanel(Entity& entity, const std::function<void(Entity&)>& func, const char* title, const char* icon) {
		// require that the componenttype is actually a component lol
		static_assert(std::is_class<T>::value, "T must be a class/struct!");

		// some flags we want for the following imgui elements
		ImGuiChildFlags childFlags = ImGuiChildFlags_AutoResizeY;
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap;

		// generate a unique id for the child window
		const std::string uniqueId = "Child" + std::to_string(typeid(T).hash_code()) + std::to_string(static_cast<uint32_t>(entity.GetHandle()));
		ImGui::BeginChild(uniqueId.c_str(), ImVec2(0, 0), childFlags);

		// this constructs the icon -> title -> and set tree visibillity
		ImGui::PushFont(Fonts::boldFont);
		float cursorY = ImGui::GetCursorPosY();
		bool isOpen = ImGui::TreeNodeEx((void*) typeid(T).hash_code(), nodeFlags, "%s", icon);
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
			entity.RemoveComponent<T>();
		}
		ImGui::EndChild();
	}

	template <typename... T>
	void ProcessAllComponents(Entity& entity, ComponentGroup<T...>) {
		(..., [&] {
			bool hasComponent = entity.HasComponent<T>();
			if (hasComponent) {
				ImGui::BeginDisabled();
			}
			const char* componentName = ComponentMetadata<T>::name;
			if (ImGui::Selectable(componentName)) {
				entity.AddComponent<T>();
			}
			if (hasComponent) {
				ImGui::EndDisabled();
			}
		}());
	}

	void Editor::OnPropertiesPanelUpdate() {
		ImGui::Begin("Properties");
		if (this->activeEntityHandle.has_value()) {

			auto& entity = this->scene->GetEntity(this->activeEntityHandle.value()); // alias active entity
			EntityHeader(entity);
			ImGui::Separator();
			ImGui::Spacing();
			ProcessComponents(entity, AllComponents{});

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


	// how to define the metadata that exists for every component

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
	struct ComponentMetadata<AudioComponent> {
		static constexpr const char* name = "Audio";
		static constexpr const char* icon = ICON_LC_AUDIO_WAVEFORM;
		static constexpr auto handler = ComponentScript;
	};
	template <>
	struct ComponentMetadata<GeometryPrimitiveComponent> {
		static constexpr const char* name = "Geometry Primitive";
		static constexpr const char* icon = ICON_LC_BLOCKS;
		static constexpr auto handler = ComponentMeshPrimitive;
	};
	template <>
	struct ComponentMetadata<GeometryGLTFComponent> {
		static constexpr const char* name = "Geometry GLTF";
		static constexpr const char* icon = ICON_LC_BLOCKS;
		static constexpr auto handler = ComponentMeshGLTF;
	};
	template <>
	struct ComponentMetadata<PointLightComponent> {
		static constexpr const char* name = "Point Light";
		static constexpr const char* icon = ICON_LC_LIGHTBULB;
		static constexpr auto handler = ComponentPointLight;
	};
	template <>
	struct ComponentMetadata<SpotLightComponent> {
		static constexpr const char* name = "Spot Light";
		static constexpr const char* icon = ICON_LC_LAMP_CEILING;
		static constexpr auto handler = ComponentSpotLight;
	};
	template <>
	struct ComponentMetadata<DirectionalLightComponent> {
		static constexpr const char* name = "Directional Light";
		static constexpr const char* icon = ICON_LC_SUN;
		static constexpr auto handler = ComponentDirectionalLight;
	};
	template <>
	struct ComponentMetadata<AmbientLightComponent> {
		static constexpr const char* name = "Ambient Light";
		static constexpr const char* icon = ICON_LC_GLOBE;
		static constexpr auto handler = ComponentEnvironmentLight;
	};

}