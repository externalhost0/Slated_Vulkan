//
// Created by Hayden Rivas on 1/11/25.
//

// external headers
#include <vulkan/vulkan_core.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

// my headers
#include <Slate/Debug.h>
#include <Slate/Files.h>
#include <Slate/Time.h>
#include <Slate/VK/vkext.h>
#include <Slate/VK/vkinfo.h>
#include <Slate/VulkanEngine.h>

// also my headers
#include "EditorGui.h"
#include "Fonts.h"

// icons!
#include <IconFontCppHeaders/IconsLucide.h>
namespace Slate {
	// forward decs
	void BuildStyle();

	static void check_vk_result(VkResult err) {
		if (err == 0) return;
		fprintf(stderr, "[Vulkan] Error: VkResult = %d\n", err);
		if (err < 0) abort();
	}


	void EditorGui::OnAttach(GLFWwindow* pNativeWindow) {
		// all imgui initialization
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGuiIO &io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
			io.ConfigDragClickToInputText = true;                // makes single click on sldiers
			io.ConfigWindowsMoveFromTitleBarOnly = true;         // makes it so you can only move windows from the bar, required for viewport functionality when undocked
			io.ConfigInputTextCursorBlink = true;                // enables blinking cursor in text boxes
#if defined(OS_MACOS)
			io.ConfigMacOSXBehaviors = true;// changes a ton of stuff, just click on it
#endif

			// 1: create descriptor pool for IMGUI
			//  the size of the pool is very oversize, but it's copied from imgui demo
			//  itself.
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


			VK_CHECK(vkCreateDescriptorPool(engine->_vkDevice, &pool_info, nullptr, &engine->imguiDescriptorPool));

			// init imgui for glfw
			ImGui_ImplGlfw_InitForVulkan(pNativeWindow, false);

			// vulkan render info from our rendersystem
			ImGui_ImplVulkan_InitInfo initInfo = {};
			initInfo.Instance = engine->_vkInstance;
			initInfo.PhysicalDevice = engine->_vkPhysicalDevice;
			initInfo.Device = engine->_vkDevice;
			initInfo.Queue = engine->_vkGraphicsQueue;

			// the pool we just created above on construction
			initInfo.DescriptorPool = engine->imguiDescriptorPool;

			initInfo.MinImageCount = 2;
			initInfo.ImageCount = 3;
			initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

			// dyn rendering requirements
			initInfo.UseDynamicRendering = true;
			initInfo.PipelineRenderingCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
					.pNext = nullptr,
					.colorAttachmentCount = 1,
					.pColorAttachmentFormats = &engine->_swapchainImageFormat};
			initInfo.Allocator = nullptr;
			initInfo.CheckVkResultFn = check_vk_result;

			// init imgui for vulkan
			ImGui_ImplVulkan_Init(&initInfo);


			// do our themes setting, imgui fonts and style
			BuildStyle();

			engine->Immediate_Submit([&](VkCommandBuffer cmd) {
				ImGui_ImplVulkan_CreateFontsTexture();
			});
		}
		// personal initialization



		// panel attach organizations
		{
			this->OnViewportAttach();
		}
	}
	void EditorGui::OnUpdate() {
		// required imgui opening
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}
		// gizmo frame
		ImGuizmo::BeginFrame();

		// below is all dockspace setup
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton;// flags for our Dockspace, which will be the whole screen

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
//					if (ImGui::MenuItem("Exit", "esc")) glfwSetWindowShouldClose(_windowRef->GetNativeWindow(), true);
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
				ImGui::Text("Frame: %u", engine->_frameNum);
				ImGuiIO &io = ImGui::GetIO();
				ImGui::Text("Platform Name: %s", io.BackendPlatformName ? io.BackendPlatformName : "Unknown Platform");
				ImGui::Text("Backend RenderManager: %s", io.BackendRendererName ? io.BackendRendererName : "Unknown RenderManager");
				ImGui::Text("Capturing Mouse: %s", io.WantCaptureMouse ? "True" : "False");
				ImGui::Text("Display Size: %.1f x %.1f", io.DisplaySize.x, io.DisplaySize.y);
				ImGui::Text("Swapchain Size: %.1u x %.1u", engine->_vkSwapchianExtent.width, engine->_vkSwapchianExtent.height);
				ImGui::Text("Image Viewport Size: %.1u x %.1u", engine->_colorImage.imageExtent.width, engine->_colorImage.imageExtent.height);
				ImGui::Text("Display Framebuffer Scale: %.1f x %.1f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
				ImGui::Text("ImGui Framerate: %.2f", io.Framerate);
				// should be the same if everything is correct
				ImGui::Text("Slate Delta Time: %.2f", Time::GetDeltaTime());
				ImGui::Text("ImGui Delta Time: %.2f", io.DeltaTime);
				ImGui::End();
			}
			ImGui::End();
		}
	}

	void EditorGui::Render(VkCommandBuffer cmd, VkImageView imageView, VkExtent2D extent2D) {
		ImGui::Render();

		VkRenderingAttachmentInfo colorattachmentinfo = vkinfo::CreateAttachmentInfo(imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingInfo renderingInfo = vkinfo::CreateRenderingInfo(extent2D, &colorattachmentinfo, nullptr);
		{
			vkext::vkCmdBeginRendering(cmd, &renderingInfo);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
			vkext::vkCmdEndRendering(cmd);
		}
	}

	void EditorGui::OnDetach(VkDevice& device) const {
		// our own gui cleanup
		{
			vkDestroySampler(device, sampler, nullptr);
		}
		// required imgui cleanup (might require waiting on device or a fence)
		{
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
	}

	void FontSetup() {
		ImGuiIO &io = ImGui::GetIO();
		// font size controls everything
		float fontSize = 17.f;

		// for crisp text on mac retina displays
		#if defined(OS_MACOS)
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
			iconFontCfg.GlyphMinAdvanceX = iconSize; // Use if you want to make the icon monospaced
			iconFontCfg.GlyphOffset = ImVec2(1.0f, fontSize/10.0f); // fixes the offset in the text
			iconFontCfg.PixelSnapH = true;
		}

		static const ImWchar ILC_Range[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};

		// the path in which all the fonts are located, figure out a better way to set this later
		const std::string path = ToDirectory("fonts/");
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
			style->WindowBorderSize = 1.0f; // for the thick borders on everything
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

			colors[ImGuiCol_MenuBarBg] = colors[ImGuiCol_WindowBg];

			// lesser colors
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
			colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);

			colors[ImGuiCol_Header] = ImVec4(0.03f, 0.03f, 0.03f, 0.72f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.22f, 0.23f, 0.6f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.53f);

			colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.49f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
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

			colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

			// HIGHLIGHT COLORS
			// colors for docking feature on hover
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.10f, 0.10f, 0.10f, 0.70f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.10f, 1.00f, 0.60f, 0.40f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.10f, 1.0f, 0.60f, 1.00f);

			colors[ImGuiCol_CheckMark] = ImVec4(0.10f, 1.0f, 0.60f, 1.00f);

			colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.10f, 1.0f, 0.60f, 1.00f);
		}
		ImGuizmo::Style* guizmostyle = &ImGuizmo::GetStyle();
		float thickness = 4.0f;
		{
			guizmostyle->TranslationLineThickness = thickness;
			guizmostyle->RotationLineThickness = thickness - 2.0f;
			guizmostyle->ScaleLineThickness = thickness;
			guizmostyle->TranslationLineArrowSize = 7.0f;
			guizmostyle->RotationOuterLineThickness = thickness - 1.0f;
			guizmostyle->HatchedAxisLineThickness = 1.0f;
		}

		ImVec4* guizmocolors = guizmostyle->Colors;
		{
			guizmocolors[ImGuizmo::DIRECTION_X] = ImVec4(0.858f, 0.243f, 0.113f, 0.929f);
			guizmocolors[ImGuizmo::DIRECTION_Y] = ImVec4(0.603f, 0.952f, 0.282f, 0.929f);
			guizmocolors[ImGuizmo::DIRECTION_Z] = ImVec4(0.227f, 0.478f, 0.972f, 0.929f);
			guizmocolors[ImGuizmo::ROTATION_USING_FILL] =  ImVec4(0.727f, 0.478f, 0.072f, 1.0f);
			guizmocolors[ImGuizmo::ROTATION_USING_BORDER] = ImVec4(0.5f, 0.3f, 0.05f, 0.8f);
		}
	}
	void BuildStyle() {
		// necessary to be done early
		FontSetup();
		// our default colors for the user interface
		StyleStandard();
	}


}