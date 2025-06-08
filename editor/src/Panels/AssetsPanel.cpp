//
// Created by Hayden Rivas on 1/16/25.
//
#include "../Editor.h"
#include "../Fonts.h"
#include "../ImGuiSnippets.h"

#include <Slate/Filesystem.h>
#include <Slate/ResourceRegistry.h>


#include <imgui_internal.h>
#include "IconFontCppHeaders/IconsLucide.h"


namespace Slate {

	const char* GetIconFromResourceType(ResourceType type) {
		switch (type) {
			case ResourceType::Shader: return ICON_LC_SPRAY_CAN;
			case ResourceType::Mesh: return ICON_LC_FILE_BOX;
			case ResourceType::Audio: return ICON_LC_AUDIO_WAVEFORM;
			case ResourceType::Font: return ICON_LC_TYPE_OUTLINE;
			case ResourceType::Script: return ICON_LC_FILE_CODE;
			case ResourceType::Image: return ICON_LC_FILE_IMAGE;
			default: return ICON_LC_FILE;
		}
	}
	bool IsHidden(const std::filesystem::path& path) {
#if defined(SLATE_OS_WINDOWS)
		DWORD attributes = GetFileAttributesW(path.wstring().c_str());
		return (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_HIDDEN);
#else
		return path.filename().string().starts_with(".");
#endif
	}

	// UNUSED //
	void OpenFileWithAssociatedApp(const std::string& filepath) {
#if defined(SLATE_OS_WINDOWS)
		ShellExecuteA(nullptr, "open", filepath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(SLATE_OS_MACOS)
		std::string cmd = "open \"" + filepath + "\"";
		system(cmd.c_str());
#elif defined(SLATE_OS_LINUX)
		std::string cmd = "xdg-open \"" + filepath + "\"";
		system(cmd.c_str());
#else
#error Platform not supported
#endif
	}


	std::string TruncateWithEllipsis(const std::string& str, float maxWidth) {
		std::string result = str;
		while (!result.empty() && ImGui::CalcTextSize(result.c_str()).x > maxWidth) {
			result.pop_back();
		}
		if (result != str) result += "...";
		return result;
	}

	void EditorApplication::_onAssetPanelUpdate() {
		ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar);
		static bool showHidden = false;
		static int thumbnail_size = 80;
		constexpr float rounding = 8.f;
		{
			if (ImGui::BeginMenuBar()) {
				bool isTopLevelDir = _currentDirectory ==_assetsDirectory;
				if (isTopLevelDir) ImGui::BeginDisabled();
				if (ImGui::MenuItem(ICON_LC_CHEVRON_LEFT)) {
					_currentDirectory = _currentDirectory.parent_path();
				}
				if (isTopLevelDir) ImGui::EndDisabled();

				if (ImGui::BeginMenu("View")) {
					ImGui::PushItemWidth(120.f);
					ImGui::MenuItem("Show Hidden Files", nullptr, &showHidden);
					ImVec2 cursorPos = ImGui::GetCursorPos();
					{
						float sliderHeight = ImGui::GetFrameHeight();
						float textHeight = ImGui::CalcTextSize("Size").y;
						float verticalOffset = (sliderHeight - textHeight) * 0.5f;
						ImGui::SetCursorPosY(cursorPos.y + verticalOffset);
					}
					ImGui::Text("Size");
					ImGui::SameLine();
					ImGui::SetCursorPosY(cursorPos.y);
					ImGui::SliderInt("###Size", &thumbnail_size, 50.f, 120.f);
					ImGui::PopItemWidth();
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Actions")) {
					if (ImGui::MenuItem("New Folder")) {
						Filesystem::CreateFolder(_currentDirectory / "NewFolder");
					}
					if (ImGui::MenuItem("Refresh NW")) {
						// Trigger refresh logic
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}
			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				_selectedEntry = std::nullopt;
			}
			const ImVec2 padding = {0.f, 5.f};
			const float cellSize = static_cast<float>(thumbnail_size) + padding.x;
			ImGui::BeginChild("Assets");
			ImGui::Spacing(); // little padding
			const float panelWidth = ImGui::GetContentRegionAvail().x;
			const float spacingX = ImGui::GetStyle().ItemSpacing.x;
			const int columnCount = std::max(1, static_cast<int>((panelWidth + spacingX) / (cellSize + spacingX)));

			ImGui::Columns(columnCount, nullptr, false);
			for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(_currentDirectory)) {
				bool hiddenItem = IsHidden(file);
				if (hiddenItem && !showHidden)
					continue;

				const std::filesystem::path& filepath = file.path();
				const std::string filename = filepath.filename().string();
				bool isDirectory = file.is_directory();
				bool isSelected = file == _selectedEntry;

				ImGui::PushID(filename.c_str());
				{

					const float full_width = static_cast<float>(thumbnail_size);
					const float full_height = static_cast<float>(thumbnail_size) + ImGui::GetTextLineHeight();
					const ImVec2 entry_size = ImVec2(full_width, full_height);
					ImGui::Spacing();
					if (hiddenItem) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.45f);
					if (ImGui::InvisibleButton("##AssetEntry", entry_size)) {
						_selectedEntry = file;
						ctx.activeEntity = std::nullopt;
					}
					if (hiddenItem) ImGui::PopStyleVar();
					// state used later
					bool hovered = ImGui::IsItemHovered();
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();

					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					ImGui::PushClipRect(min, max, false); // false = disable clipping to that region
					if (!isSelected && hovered) {
						// not selected, hovered
						draw_list->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_HeaderHovered), rounding, ImDrawFlags_RoundCornersAll);
					} else if (isSelected && !hovered) {
						// selected, not hovered
						draw_list->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_HeaderActive), rounding, ImDrawFlags_RoundCornersAll);
					} else if (isSelected && hovered) {
						// selected and hovered
						draw_list->AddRectFilled(min, max, ImGui::GetColorU32(Adjust_Brightness(ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive), 0.1f)), rounding, ImDrawFlags_RoundCornersAll);
					} else {
						// not selected not hovered
						draw_list->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_Header), rounding, ImDrawFlags_RoundCornersAll);
					}
					ImGui::PopClipRect();
					// right click popup action
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
						ImGui::OpenPopup("##ItemContextMenu");
						_selectedEntry = file;
					}

					// draw rest of entry data
					static bool isRenaming = false;
					{
						// draw icon
						const char* icon;
						if (isDirectory) {
							icon = ICON_LC_FOLDER;
						} else {
							const char* extension = filepath.extension().c_str();
							extension++; // just move the pointer forward one character removes the '.'
							icon = GetIconFromResourceType(ResolveResourceTypeFromFileExtension(extension));
						}
						ImVec2 itemMin = ImGui::GetItemRectMin();
						ImVec2 itemSize = ImGui::GetItemRectSize();
						ImVec2 iconSize = ImGui::CalcTextSize(icon, nullptr, true);
						ImVec2 iconPos = ImVec2(
								itemMin.x + (itemSize.x - iconSize.x) * 0.5f,
								itemMin.y + (itemSize.y - iconSize.y) * 0.5f
						);
						iconPos.x -= 5.0f;
						iconPos.y -= 7.0f;
						ImGui::SetCursorScreenPos(iconPos);
						ImGui::PushFont(Fonts::iconExtraLargeFont);

						ImGui::TextUnformatted(icon);
						ImGui::PopFont();

						// draw name centered
						std::string adjusted_name = TruncateWithEllipsis(filename, entry_size.x - 14.0f);
						ImVec2 textSize = ImGui::CalcTextSize(adjusted_name.c_str());
						ImVec2 textPos = ImVec2(min.x + (entry_size.x - textSize.x) * 0.5f, min.y + static_cast<float>(thumbnail_size) - 4.f);

						ImGui::SetCursorScreenPos(textPos);
						if (isRenaming && isSelected) {
							ImGui::SetKeyboardFocusHere();
							char buffer[64];
							strncpy(buffer, filename.c_str(), sizeof(buffer) - 1);
							buffer[sizeof(buffer) - 1] = '\0';
							if (ImGui::InputTextWithHint("###RenameInput", filename.c_str(), buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
								Filesystem::RenameFile(file, buffer);
								isRenaming = false;
							}
							if (ImGui::IsAnyItemFocused() && !ImGui::IsItemFocused()) {
								isRenaming = false;
							}
						} else {
							ImGui::Text("%s", adjusted_name.c_str());
						}
					}

					// right click menu
					if (ImGui::BeginPopup("##ItemContextMenu")) {
						ImGui::Text("%s: %s", isDirectory ? "Folder" : "File", filename.c_str());
						ImGui::Separator();

						if (ImGui::Selectable("Rename")) {
							isRenaming = true;
							ImGui::CloseCurrentPopup();
						}
						if (ImGui::Selectable("Delete", false, ImGuiSelectableFlags_NoAutoClosePopups)) {
							ImGui::OpenPopup("Confirm Action");
						}
						ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.1f, 0.1f, 0.15f, 0.4f));
						if (ImGui::BeginPopupModal("Confirm Action", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
							ImGui::Text("Are you sure to delete this item?");
							ImGui::PushFont(Fonts::boldFont);
							ImGui::Text("%s", filename.c_str());
							ImGui::PopFont();

							ImGui::Spacing();
							ImGui::Separator();

							if (ImGui::Button("Delete")) {
								Filesystem::Delete(file);
								ImGui::ClosePopupsExceptModals();
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel")) {
								ImGui::CloseCurrentPopup();
								ImGui::ClosePopupsExceptModals();
							}
							ImGui::EndPopup();
						}
						ImGui::PopStyleColor();
						ImGui::EndPopup();
					}


					if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						if (isDirectory) {
							_currentDirectory /= filepath.filename();
						} else {
							fmt::print("This will open the app assosciated with the file type.\n");
						}
					}

					ImGui::NextColumn();
				}
				ImGui::PopID();
			}
			ImGui::Columns(1);

			ImGui::EndChild();
		}
		ImGui::End();
	}
}