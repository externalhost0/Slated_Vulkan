
set(PROJECT_NAME "SlateEngine")
project(${PROJECT_NAME}
    LANGUAGES C CXX
)

add_library(${PROJECT_NAME}
        lib/Application.cpp
        lib/Window.cpp
        lib/Time.cpp
        lib/Input.cpp
        lib/Renderer.cpp
        lib/Files.cpp

        lib/VulkanEngine.cpp
        lib/vkinfo.cpp
        lib/vkutil.cpp
        lib/vkdescriptor.cpp
        lib/PipelineBuilder.cpp
        lib/Scene.cpp
        lib/MeshGenerators.cpp
        lib/Loader.cpp
)
# git submodules
add_subdirectory(third_party/vk-bootstrap)

# locate packages
find_package(EnTT REQUIRED)
find_package(Stb REQUIRED)
find_package(Vulkan REQUIRED) # Vulkan loads headers and glm
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(fastgltf CONFIG REQUIRED)


# include and link it
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${Stb_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC # can be found in all engine files
        Vulkan::Vulkan
        vk-bootstrap::vk-bootstrap
        EnTT::EnTT
        PRIVATE # can only be found in engine header files
        fmt::fmt
        fastgltf::fastgltf
)