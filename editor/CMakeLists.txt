
set(PROJECT_NAME "SlateEditor")
project(${PROJECT_NAME}
        LANGUAGES C CXX
)


add_executable(${PROJECT_NAME}
        src/main.cpp
        src/EditorApplication.cpp
        ${SHADERS}
        src/EditorGui.cpp
        src/PropertiesPanel.cpp
        src/ViewportPanel.cpp
        src/AssetsPanel.cpp
        src/ScenePanel.cpp
        src/ViewportCamera.cpp
        src/ImGuiSnippets.cpp
        src/ImGuiSnippets.h
        src/Context.h
)
# for compiling shaders
helper_compileShaders()
add_dependencies(${PROJECT_NAME} shaders)

find_package(imgui CONFIG REQUIRED)
find_package(ImGuizmo CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        SlateEngine
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        SlateEngine
        imgui::imgui
        imguizmo::imguizmo
)

