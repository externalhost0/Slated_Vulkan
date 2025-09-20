
set(PROJECT_NAME "SlateBoilerplate")
project(${PROJECT_NAME}
        LANGUAGES C CXX
)
add_executable(${PROJECT_NAME}
        src/main.cpp
)

# // ----- ImGui Setup ---- //
set(IMGUI_DIR "third_party/imgui")
set(IMGUI_SOURCES
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_USE_VOLK)
target_link_libraries(imgui PUBLIC SlateEngine)
# // ----- ImGuizmo Setup ---- //
set(IMGUIZMO_DIR "third_party/ImGuizmo")
set(IMGUIZMO_SOURCES
        "${IMGUIZMO_DIR}/ImGuizmo.cpp"
        "${IMGUIZMO_DIR}/ImGradient.cpp"
)
add_library(imguizmo STATIC ${IMGUIZMO_SOURCES})
target_include_directories(imguizmo PUBLIC
        ${IMGUIZMO_DIR}
)
target_link_libraries(imguizmo PRIVATE imgui)
target_compile_options(imguizmo PRIVATE -Wno-nested-anon-types)
# // --- DONE ----- //

find_package(nfd CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        SlateEngine
        PRIVATE
        third_party
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        SlateEngine
        imgui
        imguizmo
        nfd::nfd
        protobuf::libprotoc
        protobuf::libprotobuf
        protobuf::libprotobuf-lite
        PRIVATE
        "-framework IOSurface"
)

